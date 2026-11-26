# CLAUDE.md — headunit-linux

Central multimídia automotiva com sistema próprio: Linux enxuto + C++/Qt 6 (QML). Uso
pessoal (Honda HR-V). Desenvolvido e testado no PC com o hardware simulado; o CI gera o
pacote para Windows. Projeto irmão: `PedroHTF/headunit-android` (Kotlin/Compose).

## Regras de trabalho

- **Sempre em português (pt-BR):** conversa, textos da interface, comentários, commits e docs.
- O dono usa **Windows** (Qt Creator / pacote `headunit-windows` do CI). Guia:
  `docs/setup-windows.md`. Fontes em UTF-8 (`/utf-8` no MSVC).
- Precisa compilar com **Qt 6.4+** (Ubuntu 24.04 usa 6.4; CI do Windows usa 6.8). Evite APIs
  mais novas sem `#if QT_VERSION`.
- **Toda mudança de interface é conferida com capturas de tela** (ver abaixo), nos temas
  claro, escuro e alto contraste quando fizer sentido. Tela: 1024×600.
- Mantenha a paridade com o `headunit-android`: paletas (`qml/Theme.qml` ↔
  `ui/theme/Theme.kt`, mesmos valores) e geometria da câmera (`src/camera` ↔ `core/camera`).
- Hardware novo entra **atrás de uma HAL/transporte**, com versão simulada. Dependências de
  sistema (libusb, FFmpeg, ...) são **opcionais** no CMake (`HEADUNIT_HAVE_*`): o projeto
  tem que compilar e rodar sem elas.
- Atualize `README.md` (tabela "O que já funciona", opções de linha de comando) e `docs/`.

## Comandos

```bash
cmake -S . -B build -G Ninja && cmake --build build && ctest --test-dir build --output-on-failure
QT_QPA_PLATFORM=offscreen build/headunit --no-simulator --page N --screenshot arquivo.png
```

Páginas (`--page`): 0 Início, 1 Mídia, 2 Rádio, 3 Telefone, 4 Veículo, 5 Projeção,
6 Configurações. Opções úteis para capturas: `--theme light|dark|auto`, `--high-contrast`,
`--gear R`, `--steering <graus>`, `--call incoming|active`, `--equalizer`,
`--projection carplay|androidauto`, `--camera-setup`, `--media-tab 0|1|2`,
`--ignition OFF|ACC|ON`, `--radio-on`, `--splash`, `--screenshot-simulator`,
`--media <pasta>`, `--dongle usb`, `--phone auto|ofono|sim`. Todas definidas em `src/main.cpp`.

CI (`.github/workflows/build.yml`): Linux (build, testes, captura) e Windows (Qt 6.8.3
MSVC, `windeployqt`, artefato `headunit-windows` — ainda sem libusb/FFmpeg).

## Contêiner novo (nuvem)

D-Bus no Qt 6.4: conecte sinais a slots **públicos** que recebem `QDBusMessage` (slots com
`QDBusObjectPath`/`QDBusVariant` nos parâmetros são recusados). Os testes de D-Bus rodam em
`dbus-run-session` com serviços falsos (`tests/tst_ofono.cpp`).

Pacotes apt (os mesmos do CI): `qt6-base-dev qt6-declarative-dev qt6-multimedia-dev`,
módulos `qml6-module-qtquick*`, `qml6-module-qtqml-workerscript`, `qml6-module-qtmultimedia`,
`gstreamer1.0-plugins-good gstreamer1.0-libav`, `libgl-dev cmake ninja-build pkg-config`,
`libusb-1.0-0-dev libavcodec-dev libavutil-dev libswscale-dev`.

## Arquitetura

```
src/main.cpp          Monta tudo e trata a linha de comando (troque Simulated* pelos reais aqui)
src/app/              AppController (controlador central) e AppSettings
src/hal/              VehicleHal, RadioHal, PhoneHal (+ Simulated*), DrivingRules,
                      OfonoPhoneHal + PbapClient + VCard (telefone real, Qt DBus, só Linux)
src/media/            MediaLibrary (pasta = "pendrive"), FolderTree, MediaPlayerController
src/camera/           Geometria em C++ puro (CameraModel, Undistort, Guidelines,
                      SyntheticScene), CameraSettings, item QML ReverseCamera
src/projection/       Dongle Carlinkit (protocolo do node-CarPlay): DongleProtocol,
                      DongleTransport (Simulated | Usb/libusb), H264Decoder (FFmpeg),
                      ProjectionController, ProjectionView
src/audio/            Equalizer (10 bandas; hoje só salva)
src/logo/             LogoManager (logo padrão ou HeadUnit/logo.* do pendrive)
src/perf/             PerfMonitor (1º quadro, FPS, memória)
qml/                  Telas, componentes, Theme.qml, SimulatorWindow.qml
tests/                Qt Test: câmera, dongle, regras, logo/pastas, mídia, telefone/EQ, rádio
```

**AppController** decide: fonte de áudio ativa, botões do volante, ACC desligado para tudo,
ligação pausa e retoma a mídia, vídeo bloqueado acima de 5 km/h (salvo se liberado).

**Dongle:** o simulado desenha a tela do celular e responde ao toque. O real (USB) está
implementado mas **nunca foi testado com hardware** — trate qualquer mudança nele com
cuidado e documente em `docs/dongle.md`.

## Pendências conhecidas

- Microfone (Siri/Assistente e ligações pelo dongle) e multitoque na projeção.
- Telefone real: implementado (oFono/obexd, `docs/telefone.md`), falta testar com celular,
  tela de pareamento e roteamento do áudio da chamada no PipeWire.
- Equalizador real: filtro do PipeWire ou DSP da central.
- Pacote do Windows com libusb + FFmpeg para testar o dongle real.
- Redesenho do layout a partir do Figma do dono; MCU, rádio e câmera reais (dados da central).
