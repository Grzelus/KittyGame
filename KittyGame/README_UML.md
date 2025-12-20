# UML Diagram for `uml.cpp`

This repository includes a PlantUML source for the class diagram extracted from `uml.cpp`:

- `uml.puml` — PlantUML source file (placed in the `KittyGame` folder).

How to generate a PNG/SVG from `uml.puml` (PowerShell / Windows):

1. Ensure you have Java installed (Java 8+).
2. Download `plantuml.jar` (from https://plantuml.com/download) into the repo root or a tools folder.

From the repository root, run (PowerShell):

```powershell
# generate PNG
java -jar plantuml.jar -tpng .\KittyGame\uml.puml

# generate SVG
java -jar plantuml.jar -tsvg .\KittyGame\uml.puml
```

Alternative: Use the VS Code "PlantUML" extension (jebbs.plantuml) — open `uml.puml` and use the preview/Export commands.

Notes:
- The PlantUML file shows classes, main relationships (inheritance, composition, and the `Enemy -> Player` association).
- If you want changes (show private/public, add methods, or include more details), tell me which areas to expand or simplify.
