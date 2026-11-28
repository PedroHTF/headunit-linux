# Memória do projeto (passagem entre sessões)

Arquivo igual nos dois repositórios (`headunit-android` e `headunit-linux`). Leia junto com
o `CLAUDE.md` de cada um. **Atualize ao fim de cada sessão** (seção "Histórico" e "Próximos passos").

## Contexto

- Central multimídia de uso pessoal: **Honda HR-V**, central genérica **Android com Zlink**.
- Dono usa **Windows**; conversa, interface, comentários, commits e docs **em pt-BR**.
- Dois caminhos paralelos:
  - `PedroHTF/headunit-android` — Kotlin + Jetpack Compose, launcher (Android/AAOS).
  - `PedroHTF/headunit-linux` — C++/Qt 6 (QML), "sistema próprio"; CI gera pacote Windows.
- Arquitetura comum: HALs com versão simulada e real (VehicleHal, RadioHal, PhoneHal; no Linux
  também o dongle CarPlay/AA), controlador central (AppContainer / AppController).
- Toda mudança de interface é conferida com capturas de tela (comandos no `CLAUDE.md`).

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

- Trabalho desta sessão está no branch **`ccr-e1a6d4f2-sug3ud`** dos dois repositórios
  (ainda **não** foi para o `main`; nenhum pull request aberto).

## Próximos passos (lista do dono; ordem a definir com ele)

1. Microfone para Siri/Assistente e ligações pelo dongle + multitoque (Linux).
2. Telefone: **testar com celular** num PC Linux; **tela de pareamento Bluetooth** no app
   (agente BlueZ); **roteamento do áudio da chamada** no PipeWire; Android: serviço BT do fabricante.
3. Equalizador real no Linux (filtro do PipeWire ou DSP da central).
4. Pacote Windows com libusb + FFmpeg para testar o dongle real.
5. Redesenho do layout com os desenhos do Figma do dono (1024×600; claro, escuro, alto contraste) — **aguardando os desenhos**.
6. Dados da central (modelo, CPU, versão do Android, fotos da placa, `adb shell pm list packages`)
   → MCU (ACC, ré, volante, CAN), rádio real, câmera real, pacote do Zlink — **aguardando dados**.

Última sugestão feita ao dono: seguir com a tela de pareamento Bluetooth (completa o telefone).
