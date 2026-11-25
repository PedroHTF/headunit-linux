# CarPlay / Android Auto pelo dongle USB

No caminho Linux não existe Zlink: a projeção vem de um **dongle USB** do tipo Carlinkit
(VID `0x1314`, PID `0x1520`/`0x1521`). O dongle faz a autenticação com o iPhone/Android e
entrega vídeo H.264 e áudio PCM; a central só mostra o vídeo, toca o áudio e devolve os toques.

O protocolo segue a engenharia reversa do projeto aberto
[node-CarPlay](https://github.com/rhysmorgan134/node-CarPlay) (MIT): cabeçalho de 16 bytes
(`0x55AA55AA`, tamanho, tipo, ~tipo), mensagem `Open` com o tamanho do vídeo, heartbeat a cada 2 s.

> **Situação:** testado com o dongle **simulado**. Com um dongle real ainda **não foi testado**.
> Espere ajustes na primeira vez.

## Usar no PC

| Como | Comando |
|---|---|
| Dongle simulado (padrão) | `headunit` → janela Simulador → "Dongle CarPlay / Android Auto" → iPhone / Android |
| Dongle real | `headunit --dongle usb` (ou desligue "Dongle simulado" no Simulador) |
| Captura com celular simulado | `headunit --page 5 --projection carplay` |

Na tela **Projeção**, toque e arraste funcionam como na tela do celular. Na faixa da direita
ficam ⌂ (início do celular), ‹ (voltar, Android Auto) e 🎙 (Siri / Assistente). Quando o
celular toca mídia, os botões próxima/anterior/play do volante passam a controlá-lo.

## Dongle real

Precisa de duas bibliotecas na compilação (o CMake avisa se faltarem):

- **libusb 1.0**: comunicação USB. Sem ela, só o dongle simulado existe.
- **FFmpeg** (libavcodec, libavutil, libswscale): decodifica o vídeo H.264. Sem ela, o
  celular conecta mas a tela fica preta.

### Linux

```bash
sudo apt install pkg-config libusb-1.0-0-dev libavcodec-dev libavutil-dev libswscale-dev
```

Permissão para usar o dongle sem ser root (arquivo `/etc/udev/rules.d/52-carlinkit.rules`):

```
SUBSYSTEM=="usb", ATTR{idVendor}=="1314", ATTR{idProduct}=="152[01]", MODE="0666"
```

Depois: `sudo udevadm control --reload-rules` e reconecte o dongle.

### Windows

- O pacote do CI ainda **não** inclui libusb nem FFmpeg (só o dongle simulado).
- Para o dongle real será preciso compilar com libusb + FFmpeg (ex.: vcpkg) e instalar o
  driver **WinUSB** para o dongle com o [Zadig](https://zadig.akeo.ie/).

## O que ainda falta

- Microfone para Siri / Assistente e para ligações pelo CarPlay (enviar o áudio do microfone
  ao dongle).
- Multitoque (pinça para zoom nos mapas).
- Testar com dongle real e ajustar os tempos da inicialização.
