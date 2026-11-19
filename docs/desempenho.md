# Desempenho

A central é bem mais fraca que um PC (tipicamente 4–8 núcleos ARM simples e 2–4 GB de RAM).
Por isso os números do PC servem principalmente para **comparar versões** e pegar exageros.

## O que o app mede

Mostrado na tela **Veículo**, na barra superior e no Simulador:

| Métrica | O que é | Meta na central |
|---|---|---|
| **Primeiro quadro** | Tempo do início do programa até a primeira imagem na tela | < 1 s (o app); boot total < 5–8 s |
| **Quadros por segundo** | Quadros desenhados no último segundo. Parado é normal ser baixo: o Qt só redesenha quando algo muda | 60 durante animações e rolagem |
| **Memória (RAM)** | Memória residente do processo | < 150 MB tocando vídeo |

O tempo até o primeiro quadro também é impresso no console: `Primeiro quadro em 120 ms`.

## Aproximar o PC da central

### Windows
- **Limitar a 1–2 núcleos**:
  `start /affinity 3 headunit.exe` (3 = núcleos 0 e 1; 1 = só o núcleo 0).
- **Renderização por software** (pior caso, sem GPU):
  `set QT_QUICK_BACKEND=software` antes de rodar.
- **Gerenciador de Tarefas → Detalhes**: acompanhe CPU e memória do `headunit.exe`.

### Linux
```bash
taskset -c 0 ./build/headunit                         # só 1 núcleo
systemd-run --user --scope -p MemoryMax=300M ./build/headunit   # limite de RAM
QT_QUICK_BACKEND=software ./build/headunit            # sem GPU
```

## Teste mais fiel

Antes de ir para o carro, rode numa placa ARM barata (Raspberry Pi 4/5, Orange Pi) com
Linux, usando `-platform eglfs` (direto na tela, sem ambiente gráfico), como será na central.
