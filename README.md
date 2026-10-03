# Guía Técnica: Arquitectura de OpenGL 3.3 Core, Pipeline Gráfico e Interfaz con ImGui

Esta guía documenta la arquitectura estructural de una aplicación moderna en **OpenGL 3.3 Core Profile**, explicando la comunicación entre la CPU y la GPU, el funcionamiento de la memoria de video, la tubería programable (*pipeline*) y la integración de interfaces gráficas inmediatas.

---

## 1. Contexto, Ventana y Carga de Funciones

En C++ moderno no existen funciones integradas para crear ventanas en el sistema operativo ni para comunicarse directamente con el controlador de video (*driver*). Por ello, el proyecto se apoya en dos bibliotecas clave: **GLFW** y **GLAD**.

```
+---------------+        Crea la ventana y contexto        +------------------+
|  Sistema Op.  | <--------------------------------------- |      GLFW 3      |
+---------------+                                          +------------------+
        ^                                                           |
        | Provee punteros a funciones                               | glfwMakeContextCurrent
        v                                                           v
+---------------+        Carga las direcciones en runtime  +------------------+
| GPU Driver    | <--------------------------------------- |       GLAD       |
| (OpenGL 3.3)  |                                          +------------------+
+---------------+                                                   |
        ^                                                           | Provee llamadas gl*
        +-----------------------------------------------------------+

```

### El orden estricto de inclusión de cabeceras

En el código fuente, la cabecera de GLAD **siempre** debe incluirse antes que la de GLFW:

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

```

* **Motivo técnico:** `<GLFW/glfw3.h>` incluye automáticamente `<GL/gl.h>`, la cabecera estándar de OpenGL provista por el sistema operativo. Esta cabecera suele corresponder a versiones obsoletas (como OpenGL 1.1 en Windows).
* Si GLFW se incluye primero, define constantes, macros y tipos antiguos. Al incluirse GLAD posteriormente, el preprocesador y el compilador detectan redefiniciones conflictivas de tipos y funciones.
* Al incluir `<glad/glad.h>` primero, este define macros de protección que evitan que GLFW cargue las definiciones antiguas.



### Contexto gráfico y el cargador de extensiones

OpenGL opera como una **máquina de estados finita** ligada al hilo (*thread*) de ejecución actual:

1. `glfwMakeContextCurrent(window)` asocia todas las operaciones posteriores de OpenGL al contexto creado dentro de esa ventana en el hilo en curso.


2. `gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)`: Como las funciones de OpenGL residen en bibliotecas dinámicas del fabricante de la tarjeta gráfica (NVIDIA, AMD, Intel), sus direcciones de memoria deben resolverse en tiempo de ejecución. `glfwGetProcAddress` actúa como un envoltorio multiplataforma para consultar esas direcciones al sistema operativo.


3. Si intentas llamar a `gladLoadGLLoader` sin haber establecido un contexto actual previo con `glfwMakeContextCurrent`, la GPU no sabrá responder a qué contexto pertenecen las funciones y la carga fallará devolviendo punteros nulos.



---

## 2. Gestión de Memoria en GPU: VBO y VAO

La GPU procesa miles de datos en paralelo. Enviar vértices uno por uno desde la CPU durante cada fotograma satura el bus PCI Express. La arquitectura moderna exige transferir los datos a la memoria de la tarjeta gráfica (VRAM) por adelantado.

```
       MEMORIA RAM (CPU)                                MEMORIA VRAM (GPU)
+-----------------------------+                  +------------------------------+
| float vertices[] = { ... }  |                  | VBO (Buffer crudo de bytes)  |
+-----------------------------+                  | [ X, Y, Z | X, Y, Z | ... ]  |
               |                                 +------------------------------+
               | glBufferData()                                 ^
               +------------------------------------------------+
                                                                |
                                                 +------------------------------+
                                                 | VAO (Formato de lectura)     |
                                                 | Atributo 0: stride 12, off 0 |
                                                 +------------------------------+

```

### VBO (Vertex Buffer Object)

Es un arreglo lineal de bytes asignado directamente en la memoria de la GPU:

* Se reserva un identificador numérico con `glGenBuffers(1, &VBO)`.


* Se activa en la máquina de estados con `glBindBuffer(GL_ARRAY_BUFFER, VBO)`.


* Se copian los datos desde la RAM hacia la VRAM con `glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW)`.


* `GL_STATIC_DRAW` es una pista para el driver: indica que los datos se subirán una vez y se consultarán millones de veces para dibujar sin sufrir modificaciones, permitiendo a la GPU ubicarlos en la memoria de acceso más rápido.



### VAO (Vertex Array Object)

Un VBO solo contiene bytes sin formato interpretativo. El **VAO** almacena la configuración de cómo la GPU debe leer esos bytes y a qué variables del Vertex Shader debe conectarlos:

```cpp
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

```

| Parámetro | Valor | Explicación en bajo nivel|
| --- | --- | --- |
| `index` | `0` | Coincide con `layout (location = 0)` declarado en el Vertex Shader.|
| `size` | `3` | Número de componentes por vértice ($X, Y, Z$).|
| `type` | `GL_FLOAT` | Tipo de dato nativo subyacente.|
| `normalized` | `GL_FALSE` | Si es `true`, valores enteros se mapean automáticamente a $[-1.0, 1.0]$ o $[0.0, 1.0]$.|
| `stride` | `3 * sizeof(float)` | Paso o salto en bytes entre el inicio de un vértice y el inicio del siguiente.|
| `pointer` | `(void*)0` | Desplazamiento inicial (*offset*) en bytes desde el inicio del buffer.|

---

## 3. Pipeline Gráfico y Shaders en GLSL

El pipeline convierte listas de coordenadas tridimensionales en píxeles coloreados dentro de la pantalla. En OpenGL 3.3 Core Profile, este proceso requiere dos etapas programables obligatorias escritas en **GLSL** (*OpenGL Shading Language*):

```
Vértices (VBO) ---> [ Vertex Shader ] ---> Rasterización ---> [ Fragment Shader ] ---> Pantalla
                    (Calcula Posición)    (Crea fragmentos)    (Calcula Color)

```

1. **Vertex Shader:** Se ejecuta una vez por cada vértice. Su función obligatoria es calcular la posición final en coordenadas normalizadas del dispositivo (NDC) y asignarla a la variable predefinida `gl_Position`.


2. **Fragment Shader:** Tras la rasterización (donde la GPU interpola la geometría y determina qué píxeles cubre el triángulo), este shader se ejecuta por cada fragmento generado para calcular su color RGBA final.



### Variables `in` vs `uniform`

* `layout (location = 0) in vec3 aPos;`: Entrada por vértice. Cada invocación del shader recibe valores distintos según el vértice procesado.


* `uniform`: Variable global y constante para todos los vértices y fragmentos durante una llamada de dibujo (`glDrawArrays`). Se actualizan desde la CPU mediante llamadas `glUniform*` antes de invocar el renderizado.



### Compilación y enlace en tiempo de ejecución

Dado que el código binario de la GPU depende de la microarquitectura de cada fabricante, los shaders no se compilan con GCC, Clang o MSVC, sino por el propio driver de la tarjeta de video cuando la aplicación arranca:

1. `glCreateShader(...)` crea el objeto del shader.


2. `glShaderSource(...)` vincula el código fuente en texto al objeto.


3. `glCompileShader(...)` compila el código fuente a nivel de GPU.


4. `glCreateProgram()`, `glAttachShader(...)` y `glLinkProgram(...)` generan el binario unificado final (*Shader Program*).


5. `glDeleteShader(...)` libera los objetos intermedios una vez enlazados en el programa principal.



---

## 4. El Bucle de Renderizado y Doble Búfer

Una aplicación de gráficos corre en un bucle continuo hasta que el usuario cierra la ventana:

```cpp
while (!glfwWindowShouldClose(window)) {
    // 1. Limpieza de pantalla
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 2. Dibujo
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 3. Intercambio de buffers y gestión de eventos
    glfwSwapBuffers(window);
    glfwPollEvents();
}

```

* **Doble Búfer (*Double Buffering*):** Existen dos búfers de imagen: el *Front Buffer* (el que se proyecta en el monitor) y el *Back Buffer* (donde la GPU escribe la nueva imagen en segundo plano).


* Si se dibujara directamente en el buffer visible, el usuario percibiría parpadeo constante (*flickering*) y fracturas de fotograma (*screen tearing*).
* `glfwSwapBuffers(window)` intercambia instantáneamente ambos búfers cuando la GPU finaliza el dibujo completo del fotograma.


* `glfwPollEvents()` procesa los eventos del sistema operativo (clics, teclas, redimensionamiento de ventana) para evitar que la aplicación se congele.



---

## 5. Integración con GLM y Dear ImGui

Para dotar al programa de transformaciones matemáticas e interactividad, se conectan dos bibliotecas adicionales:

* **GLM (*OpenGL Mathematics*):** Biblioteca de álgebra lineal que replica la sintaxis matemática de GLSL para vectores y matrices en C++.


* **Dear ImGui:** Sistema de interfaz gráfica de modo inmediato (*Immediate Mode GUI*), ideal para depuración y paneles de control.



```
Ciclo por Fotograma:
1. Limpiar pantalla (glClear)
2. Nuevo marco de ImGui (ImGui::NewFrame)
3. Construir ventanas de controles (ImGui::Begin / End)
4. Calcular matrices con GLM y subir Uniforms (glUniform*)
5. Dibujar geometría (glDrawArrays)
6. Dibujar menú encima (ImGui::Render + RenderDrawData)
7. Intercambiar búfers (glfwSwapBuffers)

```

### Ciclo de vida de Dear ImGui

* **Inicialización (Una sola vez, antes del `while`):** Se crea el contexto (`ImGui::CreateContext()`) y se vincula con los backends de GLFW y OpenGL 3 (`ImGui_ImplGlfw_InitForOpenGL`, `ImGui_ImplOpenGL3_Init`).


* **Dentro del bucle:** Se declaran los frames de ImGui, se modela la interfaz, se dibuja la escena en OpenGL y luego se renderiza ImGui con `ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData())` para que los paneles queden superpuestos a la geometría.


* **Destrucción (Una sola vez, tras salir del `while`):** Se limpian los recursos con `ImGui_ImplOpenGL3_Shutdown()`, `ImGui_ImplGlfw_Shutdown()` y `ImGui::DestroyContext()`.



---

## 6. Errores Comunes y Puntos de Atención

* **Aserción `bd != nullptr` en ImGui:** Ocurre si colocas las funciones de apagado (`ImGui_ImplOpenGL3_Shutdown`) dentro del bucle `while`. Al terminar el primer fotograma se destruye el backend, y el segundo fotograma colapsa (*core dumped*). Las llamadas de *Shutdown* van estrictamente fuera del bucle.
* **Firma de GLAD en versiones v1:** La función `gladLoadGL(glfwGetProcAddress)` produce error de compilación en ciertas versiones. La forma correcta y estándar para inicializar el cargador con GLFW es:


```cpp
gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

```


* **Ubicación de Uniform devuelta como `-1`:** `glGetUniformLocation` devuelve `-1` si hay una falta ortográfica en el nombre de la variable o si declaraste el `uniform` en el shader pero no lo usaste en el cálculo de salida, ya que el compilador del driver lo elimina por optimización.

## 7. Chat de aprendizaje guiado

https://share.gemini.google/wgT2T9egypel