# Especificación Completa de Requisitos del Proyecto

**Proyecto #2 - Renderizado y Manipulación de Objetos 3D**  
*Universidad Central de Venezuela - Facultad de Ciencias - Escuela de Computación*  
*Introducción a la Computación Gráfica*

---

## 📌 1. Visión General del Proyecto

El objetivo es desarrollar una aplicación interactiva en **C++17**, **OpenGL 3.3 Core Profile** y **CMake** capaz de cargar, generar, renderizar, manipular e inspeccionar objetos tridimensionales en una escena con iluminación difusa (Lambert), persistencia y selección interactiva basada en hardware mediante *Color Picking*.

> [!IMPORTANT]
> Al desarrollarse el proyecto en parejas, **todos los requisitos opcionales señalados en el documento original de la cátedra pasan a ser de carácter 100% obligatorio**.

---

## 📋 2. Requisitos Obligatorios y de Parejas

### REQ-01: Carga de Mallas Tridimensionales (.obj)
* **Descripción:** La aplicación debe permitir cargar modelos poligonales 3D a partir de archivos en formato estándar Wavefront `.obj`.
* **Especificaciones Técnicas:**
  - Se permite el uso de la biblioteca de terceros `tinyobjloader`.
  - El cargador debe descomponer el objeto en sub-mallados individuales (`Mesh`) según los grupos o figuras presentes en el archivo.
  - **Aproximación de Normales:** Si el archivo no incluye vectores normales de fábrica (`vn`), la aplicación debe calcularlas automáticamente mediante el producto cruz de las aristas de cada triángulo $(\vec{v}_1 - \vec{v}_0) \times (\vec{v}_2 - \vec{v}_0)$ y promediar las normales resultantes en los vértices compartidos.
  - **Normalización de Geometría y Normales:** 
    - Al momento de la carga, el modelo debe ser centrado automáticamente en el origen $(0,0,0)$ de su espacio local.
    - Se debe escalar proporcionalmente para que sus dimensiones queden acotadas en el rango estándar $[-1, 1]$.
    - Todos los vectores normales calculados o importados deben ser normalizados a longitud unitaria ($\|\vec{n}\| = 1$).

---

### REQ-02: Carga de Propiedades de Materiales (.mtl)
* **Descripción:** Se deben cargar las propiedades asociadas al objeto desde su archivo de materiales `.mtl` complementario.
* **Especificaciones Técnicas:**
  - De dicho formato, se debe extraer y emplear exclusivamente el valor del **color difuso ($K_d$)** para tintar la superficie del objeto.
  - Si el archivo `.obj` no referencia un `.mtl` o este no existe, se debe asignar un color difuso de reserva predeterminado sin provocar errores de ejecución.

---

### REQ-03: Sistema de Selección Interactiva (Color Picking)
* **Descripción:** La selección de objetos en la escena 3D debe implementarse mediante la técnica de *Color Picking* a través de un búfer fuera de pantalla (*Off-screen Framebuffer / BackBuffer*).
* **Especificaciones Técnicas:**
  - Se realiza una pasada de renderizado oculta en un Framebuffer Object (FBO) donde cada entidad se dibuja con un color RGB plano que codifica de forma unívoca su identificador entero:
    $$\text{ID} \longleftrightarrow (R, G, B)$$
  - Al hacer clic con el mouse, se lee el píxel situado bajo el cursor con `glReadPixels` y se decodifica el ID para determinar la entidad seleccionada.
  - Se deben soportar **3 modos de especificidad de selección**:
    1. **Modo Global:** Selecciona el objeto completo (nodo raíz) junto a todos sus sub-mallados hijos.
    2. **Modo Local:** Selecciona únicamente el sub-mallado individual (`Mesh`) impactado por el cursor.
    3. **Modo Triángulo (*Requisito Parejas*):** Selecciona el triángulo específico sobre el que se hace clic. El triángulo seleccionado debe ser resaltado o marcado visualmente de manera evidente en pantalla.

---

### REQ-04: Manipulación del Objeto Seleccionado
* **Descripción:** Toda entidad seleccionada en la escena debe poder ser transformada y gestionada en tiempo real.
* **Especificaciones Técnicas:**
  - **Traslación:** Cambiar la posición tridimensional $(X, Y, Z)$ del objeto en el espacio de la escena.
  - **Rotación en el Propio Eje:** El objeto debe rotar respecto a su propio centro geométrico / eje local, evitando que orbite respecto al origen de la escena.
  - **Escala:** Alterar la escala en los tres ejes de manera independiente o uniforme.
  - **Color Difuso:** Modificar interactivamente el color difuso ($K_d$).
  - **Canal Alfa / Transparencia (*Requisito Parejas*):** Posibilidad de graduar el canal alfa ($0.0 \le \alpha \le 1.0$) del color difuso, activando el canal de mezcla de fragmentos (`glEnable(GL_BLEND)` con `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`).
  - **Eliminación:** Eliminar el objeto seleccionado de la escena liberando sus recursos asociados.
  - **Propagación Jerárquica:** Cualquier transformación aplicada sobre un objeto en modo Global debe propagarse automáticamente y de forma coherente a todos sus sub-mallados asociados.

---

### REQ-05: Modos de Visualización e Inspección Geométrica
* **Descripción:** Sobre el objeto o sub-mallado actualmente seleccionado, el usuario debe poder activar o desactivar de forma independiente cuatro modos de visualización:
* **Especificaciones Técnicas:**
  - **Modo Wireframe:** Conmutar el rasterizado de caras a líneas visibles (`glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)`).
  - **Visualizar / Ocultar Normales:** Dibujar líneas vectoriales (`GL_LINES`) que partan desde los vértices en la dirección de sus respectivos vectores normales.
  - **Visualizar / Ocultar Vértices:** Dibujar la nube de puntos que compone la malla usando `GL_POINTS` con un tamaño de punto claramente distinguible (`glPointSize`).
  - **Visualizar / Ocultar Bounding Box:** Dibujar la caja envolvente delimitadora (AABB - Axis-Aligned Bounding Box) del objeto, la cual debe recalcularse y transformarse solidariamente con el modelo.

---

### REQ-06: Control de Estados del Pipeline Gráfico de OpenGL
* **Descripción:** La aplicación debe permitir conmutar en tiempo real mediante la interfaz dos estados esenciales de descarte y prueba de fragmentos:
* **Especificaciones Técnicas:**
  - Conmutador para activar / desactivar la prueba de profundidad: **Depth Test** (`glEnable(GL_DEPTH_TEST)` / `glDisable(GL_DEPTH_TEST)`).
  - Conmutador para activar / desactivar el descarte de caras traseras: **Back-Face Culling** (`glEnable(GL_CULL_FACE)` / `glDisable(GL_CULL_FACE)`).

---

### REQ-07: Sistema de Cámara Libre (Navegación WASD + Mouse)
* **Descripción:** Implementar una cámara en primera persona que permita explorar y desplazarse por el espacio 3D de la escena.
* **Especificaciones Técnicas:**
  - **Teclas WASD:** Movimiento traslacional de la cámara (adelante, atrás, izquierda, derecha, más controles opcionales para elevación).
  - **Mouse:** Control de la orientación angular (Yaw y Pitch) con límites en el cabeceo para evitar volteos de cámara (acotado a $[-89^\circ, 89^\circ]$).
  - Generación de las matrices matemáticas correspondientes:
    - Matriz de Vista (`View`): Construida mediante `glm::lookAt`.
    - Matriz de Proyección (`Projection`): Proyección en perspectiva basada en el campo de visión (FOV) y la relación de aspecto de la ventana.

---

### REQ-08: Primitivas Geométricas Parametrizables
* **Descripción:** El usuario debe poder instanciar en la escena figuras geométricas elementales generadas matemáticamente por código, sin recurrir a archivos externos.
* **Figuras requeridas:**
  1. **Cubo:** Parametrizable en tamaño de arista.
  2. **Pirámide:** Parametrizable en base y altura.
  3. **Esfera:** Parametrizable en radio, número de sectores y anillos (stacks).
  4. **Cilindro (*Requisito Parejas*):** Parametrizable en radio, altura y segmentos de revolución.
* **Comportamiento:** Una vez creadas, estas primitivas se comportan exactamente igual a cualquier otro objeto de la escena (pueden seleccionarse, escalarse, rotarse, eliminarse, inspeccionar sus normales/bounding box, etc.).

---

### REQ-09: Control del Entorno y Rendimiento de la Escena
* **Descripción:** La aplicación debe proporcionar controles globales de gestión del entorno visual:
* **Especificaciones Técnicas:**
  - **Borrado Total de la Escena:** Opción para vaciar la lista de objetos y reiniciar el estado de la escena.
  - **Color de Fondo:** Selector interactivo de color para modificar en tiempo real el valor de limpieza de la pantalla (`glClearColor`).
  - **Métrica de Rendimiento:** Mostrar de forma permanente y visible en la interfaz gráfica el contador de fotogramas por segundo (**FPS**) y tiempo por cuadro (*frametime* en ms).

---

### REQ-10: Persistencia de Escena (Guardar y Cargar)
* **Descripción:** La escena completa debe poder almacenarse en el disco y ser recuperada en ejecuciones posteriores.
* **Especificaciones Técnicas:**
  - **Guardar Escena:** Exportar a un archivo de texto o JSON la lista completa de objetos presentes, sus tipos (primitiva o ruta al `.obj`), sus matrices de transformación (posición, rotación, escala), colores difusos, canal alfa, parámetros de iluminación global y configuración del fondo.
  - **Cargar Escena:** Leer el archivo previamente guardado, reconstruir los nodos de la escena, regenerar/cargar sus geometrías y restaurar todos los parámetros de transformación de manera idéntica.

---

### REQ-11: Integración de Shaders Provistos (Lambert)
* **Descripción:** El renderizado principal debe emplear el código base de Vertex Shader y Fragment Shader con iluminación difusa (Lambert) suministrado por la cátedra.
* **Especificaciones Técnicas:**
  - **Restricción estricta:** No se debe modificar la formulación matemática del modelo de iluminación en los shaders.
  - La aplicación en C++ es enteramente responsable de compilar, enlazar y alimentar correctamente todos los atributos requeridos (`aPos`, `aNormal`) y las variables de control (*uniforms*: matrices MVP, posición y color de la fuente de luz, color difuso y coeficiente alfa).

---

## 📦 3. Requisitos de Entrega y Evaluación
* **Lenguaje y Construcción:** C++17 con CMake versión 3.20 o superior.
* **Nomenclatura del entregable:** Archivo comprimido `.zip` nombrado exactamente `PROY2_CEDULA1_CEDULA2.zip`.
* **Defensa Técnica:** El proyecto cuenta con una defensa oral obligatoria en la que ambos integrantes deben explicar las decisiones arquitectónicas, matemáticas y de implementación adoptadas.
