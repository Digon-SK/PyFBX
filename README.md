# PyFBX

Wrapper moderno, tipado y pitónico de [ufbx](https://github.com/ufbx/ufbx), un lector FBX
rápido y robusto escrito en C. La extensión nativa usa `ufbx` 0.23.0 y mantiene su API de
bajo nivel encapsulada detrás de objetos Python seguros.

## Características

- `load()` acepta `str` y `pathlib.Path`; `loads()` acepta bytes y `memoryview`.
- Objetos navegables: `Scene`, `Node`, `Mesh`, `Material`, `Texture` y `AnimationStack`.
- Ownership automático: conservar un `Mesh` o `Node` conserva también la escena nativa.
- Polígonos originales y triangulación coherente con atributos por esquina (UV seams).
- Transformaciones como matrices 4×4 convencionales.
- Evaluación de animaciones y skinning.
- Curvas, keyframes, skin weights y blend shapes accesibles directamente.
- Cámaras, luces, huesos, propiedades personalizadas y elementos poco comunes.
- NURBS y topología half-edge para flujos de geometría avanzada.
- Conversión opcional de ejes y unidades durante la carga.
- NumPy opcional, sin convertirlo en dependencia obligatoria.
- Type hints incluidos mediante `py.typed` y `_native.pyi`.

## Instalación

```bash
python -m pip install .
```

Para desarrollo:

```bash
python -m pip install -e ".[test]"
python -m pytest
```

`scikit-build-core` descarga las herramientas de build declaradas y compila la extensión.
El código de `ufbx` está incluido y fijado a 0.23.0, por lo que el build no depende de clonar
su repositorio.

## Uso rápido

```python
from pathlib import Path

import pyfbx

scene = pyfbx.load(Path("character.fbx"))
print(scene, scene.creator)

for node in scene:
    print(node.path, node.attribute_type)
    if mesh := node.mesh:
        print(mesh.num_vertices, mesh.num_triangles)
        print(mesh.triangles[:3])

hips = scene.find_node("Hips")
if hips is not None:
    print(hips.node_to_world)
```

### Carga normalizada

```python
options = pyfbx.LoadOptions(
    target_axes=pyfbx.AxisSystem.RIGHT_HANDED_Y_UP,
    target_unit_meters=1.0,
    evaluate_skinning=True,
)
scene = pyfbx.load("character.fbx", options=options)
```

### Geometría y NumPy

`mesh.vertices` contiene posiciones lógicas únicas y `mesh.faces` usa sus índices.
Para render o exportación, `mesh.vertex_positions`, `mesh.normals`, `mesh.uvs` y
`mesh.colors` están alineados por esquina. `mesh.triangle_indices` indexa esas matrices:

```python
arrays = pyfbx.as_numpy(scene.meshes[0])
positions = arrays["positions"]
triangles = arrays["triangles"]
render_positions = positions[triangles]
```

`as_numpy()` realiza una copia deliberada: los arrays no quedan ligados al lifetime de la
escena y pueden modificarse con seguridad.

### Animación

```python
take = scene.animations[0]
frame = scene.evaluate(take.time_begin + 0.5, take)
print(frame["Hips"].node_to_world)
```

## Seguridad

`load_external_files=False` es el valor predeterminado. Actívalo solamente para archivos de
confianza: FBX puede contener referencias a rutas externas. `memory_limit_bytes` y
`node_depth_limit` permiten acotar entradas no confiables.

## Estructura

```text
src/pyfbx/          API pública, opciones, helpers y type hints
src/native/         frontera C++/Python organizada por dominio
third_party/ufbx/   fuente oficial fijada y su licencia MIT
tests/              pruebas funcionales y un FBX ASCII pequeño
examples/           scripts ejecutables
```

La API pública se importa desde `pyfbx`; `_native` es un detalle de implementación.

La referencia resumida está en [`docs/api.md`](docs/api.md) y las instrucciones para
contribuir en [`CONTRIBUTING.md`](CONTRIBUTING.md).
