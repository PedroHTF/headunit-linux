# Ambiente de desenvolvimento no Windows

## 1. Instalar o Qt

1. Baixe o **Qt Online Installer** em <https://www.qt.io/download-qt-installer-oss>
   (precisa criar uma conta Qt gratuita; escolha a licença *open source*).
2. Na seleção de componentes, marque:
   - **Qt 6.8.x** (LTS) → **MSVC 2022 64-bit** *ou* **MinGW 64-bit**
     (MinGW é mais simples: não precisa do Visual Studio)
   - dentro do Qt 6.8.x → **Additional Libraries → Qt Multimedia**
   - **Developer and Designer Tools**: Qt Creator, CMake, Ninja e (se escolheu MinGW) o MinGW correspondente.
3. Se escolheu **MSVC**, instale também o *Visual Studio 2022 Build Tools* com
   "Desenvolvimento para desktop com C++".

## 2. Abrir e rodar

1. Clone o repositório (GitHub Desktop ou `git clone https://github.com/PedroHTF/headunit-linux`).
2. Qt Creator → **File → Open File or Project** → escolha o `CMakeLists.txt`.
3. Selecione o *kit* (Qt 6.8 MinGW ou MSVC) → **Configure Project**.
4. Botão **Run ▶** (Ctrl+R). Abrem as janelas **HeadUnit** e **Simulador**.

Para passar opções (ex.: `--media D:\Musicas`): **Projects → Run → Command line arguments**.

## 3. Testes

No Qt Creator: **Tools → Tests → Run All Tests**, ou no terminal:

```bat
ctest --test-dir build --output-on-failure
```

## 4. Simular o pendrive

- A pasta padrão é `%USERPROFILE%\HeadUnitUSB` (criada na primeira execução).
  O botão **Abrir pasta** no Simulador abre no Explorer.
- Copie músicas (mp3, flac, ogg, wav, m4a...) e vídeos (mp4, mkv, avi, webm...).
  Subpastas viram o "artista/álbum" na lista.
- O interruptor **Pendrive USB** no Simulador simula tirar e colocar o pendrive.

## 5. Ver como fica em tela cheia

`headunit --fullscreen` (saia com Alt+F4).

## Problemas comuns

| Sintoma | Solução |
|---|---|
| "Could not find a package configuration file provided by Qt6Multimedia" | Instale **Qt Multimedia** no Qt Maintenance Tool |
| Vídeo não toca / sem som | Confirme o Qt 6.5+ (usa FFmpeg no Windows); veja o erro mostrado na tela Mídia |
| Janela do Simulador sumiu | Ela abre à direita da principal; arraste a principal ou rode sem `--no-simulator` |
