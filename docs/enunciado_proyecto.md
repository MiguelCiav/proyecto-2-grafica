# Enunciado Oficial del Proyecto

**Universidad Central de Venezuela**  
**Facultad de Ciencias**  
**Escuela de Computación**  
**Licenciatura en Computación**  
*Introducción a la Computación Gráfica* — *junio 2026*

---

## Proyecto #2 - Renderizado y Manipulación de Objetos 3D

Se les ha encomendado la misión de implementar una aplicación que permita cargar, renderizar y manipular objetos en 3D.

---

### Requisitos Obligatorios
*Estos requisitos son obligatorios para todos los grupos.*

* **Cargar Objetos 3D desde un archivo en formato “.obj”.**
  * En este punto puede hacer uso de la librería “TinyObjLoader”.
  * En caso de que el objeto no incluya normales propias, estas deben aproximarse usando el promedio de las normales de cada vértice que forma un triángulo.
  * Al cargar, se debe normalizar el objeto y sus normales.

* **Cargar las propiedades del objeto desde un archivo en formato “.mtl”** *(indicado en enunciado original como .mlt)*.
  * De dicho formato solo se deberá usar el color difuso (kd) como color para pintar todo el objeto.

* **Se deben poder seleccionar objetos en 2 tipos de especificidad:**
  * **Local:** Este modo selecciona únicamente el sub-mallado.
  * **Global:** Este modo selecciona el objeto entero, junto a sub-mallados.
  * Dicha selección debe estar implementada usando **color picking**. Para ello puede hacer uso de BackBuffer o un Framebuffer, al pintar todos los objetos con un color único.

* **Del objeto seleccionado se deben poder alterar las siguientes propiedades:**
  * Cambiar de posición.
  * Rotar respecto al propio eje.
  * Alterar color Difuso.
  * Eliminar el objeto.
  * Alterar escala del objeto.
    * Además se debe poder visualizar el objeto en modo Wireframe.
    * Visualizar/Ocultar sus normales.
    * Visualizar/Ocultar sus vértices.
    * Visualizar/Ocultar su Bounding Box.
  * Se espera que cualquier cambio realizado sobre un objeto afecte a sus sub-mallados asociados.

* **Se debe poder activar y desactivar el Depth Test y el Back-Face Culling.**

* **Se debe implementar una cámara simple que permite desplazarse por la escena usando el mouse y las teclas WASD.**

* **Toda la escena, incluyendo objetos cargados y sus cambios, se debe poder guardar y cargar posteriormente.**

* **Se deben poder crear objetos simples parametrizables: Pirámide, Cubo, Esfera.** Una vez creados, estos objetos actuarán como cualquier otro objeto en escena.

* **Se debe borrar la escena completa, alterar el color de fondo, y visualizar los FPS actuales.**

---

### Requisitos Opcionales
*Estos requisitos son de carácter opcional para los proyectos individuales, y se traducen en puntos extra sobre la nota otorgada por los requisitos obligatorios (con un máximo de 20 pts). **Para los proyectos en parejas son de carácter totalmente obligatorio**.*

* **Alterar el canal alfa del color difuso de los objetos para generar transparencias.**
* **La selección debe contar con un tercer modo de especificidad: seleccionar por triángulo.** Este debe ser marcado de alguna manera visual.
* **Se debe incluir el cilindro en la lista de objetos parametrizables.**

> **Nota sobre Shaders:**  
> Se proporcionará un código base de Vertex y Fragment Shader que incluye un modelo de iluminación difusa (Lambert). Los estudiantes no deberán modificar la matemática de iluminación de este shader, pero son responsables de compilarlo, enlazarlo y enviarle correctamente los atributos necesarios.

---

### CONDICIONES GENERALES

* El proyecto debe ser desarrollado en el lenguaje de programación **C++ y CMake**. O con el lenguaje de programación Kotlin y Maven/Gradle.
* El proyecto puede ser entregado en parejas, o de forma individual.
* La fecha de entrega queda pautada para el día **02 de octubre de 2026 hasta las 11:59 PM (GMT-4)**.
* Todos los proyectos tendrán una **defensa asociada**, donde los desarrolladores deberán exponer su solución y explicar sus decisiones de diseño.
* El código debe ser entregado en un zip con el formato siguiente:
  ```text
  PROY2_CEDULA1_CEDULA2.zip
  ```
  a `bryansilva.dev@gmail.com`, con el asunto replicando el nombre del archivo `.zip` en el correo.
