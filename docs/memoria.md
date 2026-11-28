# Memória do projeto (passagem entre sessões)

Arquivo igual nos dois repositórios (`headunit-android` e `headunit-linux`). Leia junto com
o `CLAUDE.md` de cada um. **Atualize ao fim de cada sessão** (seção "Histórico" e "Próximos passos").

## Contexto

- Central multimídia de uso pessoal: **Honda HR-V**, central **H-Tech HT-9864CA** (Android, Unisoc UIS8581A, Zlink).
- Dono usa **Windows**; conversa, interface, comentários, commits e docs **em pt-BR**.
- Dois caminhos paralelos:
  - `PedroHTF/headunit-android` — Kotlin + Jetpack Compose, launcher (Android/AAOS).
  - `PedroHTF/headunit-linux` — C++/Qt 6 (QML), "sistema próprio"; CI gera pacote Windows.
- Arquitetura comum: HALs com versão simulada e real (VehicleHal, RadioHal, PhoneHal; no Linux
  também o dongle CarPlay/AA), controlador central (AppContainer / AppController).
- Toda mudança de interface é conferida com capturas de tela (comandos no `CLAUDE.md`).

## Hardware da central (informado pelo dono em 2026-11-28)

- Modelo: **H-Tech HT-9864CA** (marca brasileira H-Tech), 2 DIN, tela 9" QLED.
- Dados de anúncios e reclamações (**não confirmados no aparelho**):
  - Plataforma **Unisoc UIS8581A** (família do SC9863A: 8 núcleos Cortex-A55 a 1,6 GHz,
    GPU PowerVR GE8322). Fonte: relato no Reclame Aqui; os anúncios só dizem "octa-core 1,6 GHz".
  - 4 GB de RAM e 64 GB de armazenamento. Algumas unidades parecidas são anunciadas com 6/128 GB.
  - Anunciada como "Android 13", mas há reclamação de que a versão mostrada não é a real
    (é comum nessas centrais: Android 10 ou 11 com o nome trocado). **Confirmar** com
    `adb shell getprop ro.build.version.sdk` (o app pede no mínimo `minSdk 28`, Android 9: serve de qualquer jeito).
  - Resolução **1280×720 ou 1280×800** (os anúncios não batem). **Não é 1024×600**, que é a
    tela de referência atual dos dois projetos.
  - Áudio 4×60 W RMS (anunciado), saída para subwoofer, 2 USB, câmera de ré e câmera frontal,
    controle de volante por 2 fios (SWC, até 10 funções), GPS, Bluetooth, Wi-Fi,
    CarPlay/Android Auto com e sem fio (pelo app de espelhamento, provavelmente o Zlink).
  - Sem firmware oficial publicado e suporte da fabricante fraco (Reclame Aqui): **não dá
    para contar com atualização de sistema**; o launcher tem que se virar com o sistema que vier.
- Ainda falta descobrir no aparelho: resolução e densidade (`adb shell wm size` / `wm density`),
  versão real do Android, fabricante da placa e do MCU (Configurações → Sobre/Informações
  do sistema, versão do MCU), lista de pacotes (`adb shell pm list packages`: serviço de
  Bluetooth, app do Zlink, app de rádio, serviço do MCU) e como o MCU conversa com o Android
  (porta serial, serviço do fabricante ou broadcasts).

## Situação das funções

| Função | Android | Linux |
|---|---|---|
| Mídia do pendrive (com pastas), logo de inicialização, temas claro/escuro/auto + alto contraste | ✅ | ✅ |
| Rádio FM/AM | 🟡 simulado | 🟡 simulado |
| Câmera de ré calibrável (fisheye, linhas em m e "carros", trajetória pelo volante) | ✅ (imagem simulada) | ✅ (imagem simulada) |
| Equalizador 10 bandas | ✅ aplicado (AudioEffects) | 🟡 só salvo |
| Telefone | 🟡 simulado (falta serviço BT do fabricante) | ✅ real via oFono/obexd, **não testado com celular** |
| Android Auto / CarPlay | ✅ abre o Zlink (auto só na 1ª conexão BT da viagem) | ✅ dongle simulado · 🟡 dongle USB real nunca testado |

## Histórico de sessões

### 2026-11-26 a 28
- Criado `CLAUDE.md` nos dois repositórios.
- **Linux — telefone real** (commit "Telefone real no Linux: oFono (HFP) e PBAP (obexd)"):
  - `src/hal/OfonoPhoneHal` (chamadas HFP pelo oFono, D-Bus do sistema, tudo assíncrono:
    receber/atender/recusar/ligar, chamada em espera via HoldAndAnswer, mudo, DTMF, sinal, bateria).
  - `src/hal/PbapClient` (agenda "pb" e histórico "cch" pelo obexd, D-Bus de sessão).
  - `src/hal/VCard` (vCard 2.1/3.0, QUOTED-PRINTABLE, linhas dobradas, X-IRMC-CALL-DATETIME).
  - `--phone auto|ofono|sim` (+ variável `HEADUNIT_PHONE`); opção CMake `HEADUNIT_PHONE_DBUS`.
  - Tela Telefone: estado da agenda, botão "Atualizar agenda", lista vazia explicada.
  - Testes `tst_vcard` e `tst_ofono` (oFono e obexd falsos em `dbus-run-session`).
  - Doc: `docs/telefone.md` (montagem BlueZ/oFono/obexd/PipeWire, pareamento, problemas).
- Android: nada mudou além do `CLAUDE.md`.

### 2026-11-28 (sessão seguinte)
- Dono informou o modelo da central: **H-Tech HT-9864CA**. Ficha técnica pesquisada e
  registrada em "Hardware da central" (nada de código mudou).

## Lições técnicas (não repetir o erro)

- **QtDBus no Qt 6.4:** `QDBusConnection::connect` **recusa** slots com `QDBusObjectPath` ou
  `QDBusVariant` nos parâmetros (retorna `false` sem erro). Use slots **públicos** que recebem
  `const QDBusMessage &` e leia `msg.arguments()` / `msg.path()` / `msg.member()`.
- No Linux o Qt Gui depende do Qt DBus: para testar "sem D-Bus" use `-DHEADUNIT_PHONE_DBUS=OFF`
  (não `CMAKE_DISABLE_FIND_PACKAGE_Qt6DBus`, que quebra o Qt Quick).
- Captura com celular "falso" conectado:
  `dbus-run-session -- bash -c 'export DBUS_SYSTEM_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS;
  HEADUNIT_FAKE_OFONO_SECONDS=10 build/tst_ofono serveFakesForScreenshots & sleep 1;
  QT_QPA_PLATFORM=offscreen build/headunit --no-simulator --phone ofono --page 3 --screenshot x.png'`
  (`HEADUNIT_FAKE_OFONO_CALL=1` faz chegar uma ligação).
- Contêiner novo: Qt pelo apt (lista no `CLAUDE.md` do Linux); Android SDK em `/opt/android-sdk`
  (+ `local.properties`); se o Robolectric der 429, jars de repo1.maven.org em `~/.m2` + init
  script do Gradle com `mavenLocal()` primeiro.

## Branches

- Sessão 2026-11-26 a 28: branch **`ccr-e1a6d4f2-sug3ud`**. Sessão seguinte: **`ccr-6d1ecc31-bf035c`**,
  criado em cima dele. Nenhum dos dois foi para o `main`; nenhum pull request aberto.

## Próximos passos (lista do dono; ordem a definir com ele)

1. Microfone para Siri/Assistente e ligações pelo dongle + multitoque (Linux).
2. Telefone: **testar com celular** num PC Linux; **tela de pareamento Bluetooth** no app
   (agente BlueZ); **roteamento do áudio da chamada** no PipeWire; Android: serviço BT do fabricante.
3. Equalizador real no Linux (filtro do PipeWire ou DSP da central).
4. Pacote Windows com libusb + FFmpeg para testar o dongle real.
5. Redesenho do layout com os desenhos do Figma do dono (tela a definir, ver item 7; claro, escuro, alto contraste) — **aguardando os desenhos**.
6. Central = **H-Tech HT-9864CA** (ver "Hardware da central"). Falta confirmar no aparelho:
   resolução, versão real do Android, MCU e `adb shell pm list packages`
   → MCU (ACC, ré, volante, CAN), rádio real, câmera real, pacote do Zlink.
7. Decidir com o dono se a tela de referência passa de 1024×600 para **1280×720/800**
   (capturas, emulador e desenhos do Figma).

Última sugestão feita ao dono: seguir com a tela de pareamento Bluetooth (completa o telefone).
