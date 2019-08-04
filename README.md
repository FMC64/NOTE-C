# NOTE-C
C IDE (editor, compiler, linker) for Casio SH3 / SH4 calculators

## Supported features
- Work in progress

## Incoming features
- C89 compiler
- Linker (got that from a 2017 project)
- Bitmap editor (same, not integrated yet)
- Hex editor (to explore root)

## Potential features
- Syntax highlighting
- Code completion (procedures, type, variable names suggestion)

## How to build

- Make sure the `Fx-9860G SDK` is installed on your system (Windows only)
  - Official long way: https://edu.casio.com/products/graphic/fx9860g2/ _(Support -> Download Resources -> fx-9860GII series -> SDK)_
  - Unofficial short way: https://www.planet-casio.com/Fr/logiciels/voir_un_logiciel_casio.php?showid=76 _(sdk85.rar)_
  - **Make sure the SDK path does not contain any space !**
- Open project file `NOTEC.g1w` with the SDK
- Build the project `Project -> Build`
- That's it ! Now you can test it with the integrated emulator and send it (`NOTEC.G1A`) right to your calculator =D
