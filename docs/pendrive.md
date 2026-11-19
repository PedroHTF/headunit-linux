# Como organizar o pendrive

No PC, o "pendrive" é a pasta `HeadUnitUSB` (ou a passada em `--media`). Qualquer
estrutura de pastas funciona. A pasta `HeadUnit` é reservada para arquivos do sistema,
como o logo, e não aparece na lista de mídia.

```
PENDRIVE/
├── HeadUnit/
│   └── logo.png        ← logo da inicialização (ou logo.jpg, logo.webp, logo.mp4)
├── Musicas/
│   ├── Rock/
│   │   └── ... .mp3
│   └── MPB/
│       └── ... .flac
└── Videos/
    └── ... .mp4
```

## Mídia

- **Mídia → Pastas**: navegue pelas pastas (‹ volta). Tocar num arquivo toca a pasta
  inteira a partir dele, músicas e vídeos.
- As abas **Músicas** e **Vídeos** mostram tudo, sem pastas.

## Logo da inicialização

1. Coloque `HeadUnit/logo.png` (resolução da tela, fundo escuro) ou `logo.mp4` (sem som,
   até 15 s).
2. **Configurações → Logo de inicialização → Procurar no pendrive** e toque no arquivo.
   Ou **Escolher arquivo** para pegar de qualquer lugar.
3. O arquivo é copiado para os dados do app: o pendrive pode sair depois.
4. **Usar logo padrão** volta ao logo do app ("HR-V" cromado com reflexo em terracota).
