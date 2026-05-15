# Resumen exhaustivo de Robótica Móvil y Proyecto Final

Este documento resume el contexto de la asignatura **RA: Robótica Móvil 2026**, las prácticas entregadas en clase, los conceptos ROS trabajados, los comandos importantes, los nodos principales y el estado general del proyecto final dentro de este repositorio.

Fuentes usadas:

- `Context/P1 RM- Recordant ROS.pdf`
- `Context/P2 RM- Topics de ROS.pdf`
- `Context/P3-RM_ Navegació.pdf`
- `Context/P4-RM Mapeig i Localitzacií.pdf`
- `Context/RA- EnunciatProjecteFinal-2026-RM.pdf`
- Código local de la solución de la práctica 2 en `Context/solucio_P2 RM- Topics de RO/`

---

## 1. Contexto general de la asignatura

La asignatura trabaja la robótica móvil usando **ROS**, **TurtleBot3**, **Gazebo** y **RViz**. El objetivo práctico es aprender a controlar un robot móvil, leer sensores, construir mapas, localizar el robot y usar el stack de navegación para ir de un punto a otro evitando obstáculos.

El flujo global de aprendizaje es:

1. Recordar ROS y su estructura básica.
2. Crear paquetes, nodos y launch files.
3. Publicar y suscribirse a topics.
4. Controlar el robot mediante `/cmd_vel`.
5. Leer sensores como `/scan` y `/odom`.
6. Simular TurtleBot3 en Gazebo.
7. Visualizar datos en RViz.
8. Crear mapas mediante SLAM/gmapping.
9. Guardar y servir mapas con `map_server`.
10. Localizar el robot con AMCL.
11. Planificar rutas y evitar obstáculos con `move_base`.
12. Aplicar todo esto al proyecto final.

La robótica móvil del curso está centrada en robots de conducción diferencial, especialmente **TurtleBot3 Burger** en prácticas y **TurtleBot3 Waffle Pi** en el proyecto final.

---

## 2. Evaluación y proyecto final

La parte de Robótica Móvil representa el **50% de la nota final** de la asignatura:

| Elemento | Peso |
|---|---:|
| NP2, Nota Parcial 2 | 30% |
| NPF, Nota Proyecto Final | 15% |
| ET, Entregable Teoría | 5% |

La nota se evalúa de 0 a 10. Los grupos pueden tener un máximo de 5 personas.

### 2.1. Objetivo del proyecto final

El proyecto final consiste en desarrollar e implementar algoritmos de navegación/control para una plataforma robótica móvil. El robot de referencia indicado en el enunciado es:

- **TurtleBot3 Waffle Pi**

El proyecto debe usar como mínimo estos topics:

| Topic | Uso |
|---|---|
| `/cmd_vel` | Enviar velocidades lineales y angulares al robot |
| `/images` | Leer imágenes de cámara |
| `/scan` | Leer datos del LIDAR |

### 2.2. Puesta en marcha del robot real

Comandos indicados para el TurtleBot3 Waffle Pi:

```bash
export TURTLEBOT3_MODEL=waffle_pi
roscore
ssh ubuntu@IP_ROBOT
roslaunch turtlebot3_bringup turtlebot3_robot.launch
ssh ubuntu@IP_ROBOT
roslaunch turtlebot3_bringup turtlebot3_rpicamera.launch
roslaunch turtlebot3_bringup turtlebot3_remote.launch
```

### 2.3. Entrega/presentación del proyecto

La presentación debe incluir:

- Descripción del proyecto.
- Solución desarrollada.
- Vídeos de funcionamiento.
- Demo en directo, ya sea en simulación o con robot real.

### 2.4. Ideas propuestas en el enunciado

El enunciado propone ejemplos de proyectos:

1. **Navegación visual**: seguir caminos marcados con líneas o flechas mientras se evitan obstáculos dinámicos usando `/scan`.
2. **Mapas semánticos**: usar SLAM y añadir identificación/etiquetado de objetos como personas o sillas.
3. **Seguidor inteligente**: seguir a una persona concreta ignorando otras.
4. **Sistema de distancia social**: patrullar una zona y detectar exceso de personas o distancias inseguras.
5. **Guía inteligente de museo**: navegar a puntos de interés cuando una persona se aproxima o mira.
6. **Asistente de inventario**: recorrer pasillos y contar objetos de un tipo concreto.

---

## 3. Conceptos base de ROS

ROS organiza los programas robóticos en una arquitectura distribuida de nodos que se comunican mediante topics, servicios, parámetros y transformaciones.

### 3.1. Nodo

Un **nodo** es un proceso ejecutable de ROS. Cada programa que hace una tarea concreta suele ser un nodo:

- Un nodo que lee el LIDAR.
- Un nodo que publica velocidades.
- Un nodo de localización.
- Un nodo de SLAM.
- Un nodo de navegación.

Comandos:

```bash
rosnode list
rosnode info /nombre_del_nodo
```

Ejemplo de nodo mínimo en C++:

```cpp
#include <ros/ros.h>

int main(int argc, char** argv) {
  ros::init(argc, argv, "Remembering_ROS");
  ros::NodeHandle nh;
  ROS_INFO("Trying to remember ROS...");
  ros::spinOnce();
  return 0;
}
```

Si el nodo termina enseguida, no aparece de forma persistente en `rosnode list`. Para mantenerlo activo se usa un bucle:

```cpp
ros::Rate loop_rate(2);

while (ros::ok()) {
  ROS_INFO("Node running...");
  ros::spinOnce();
  loop_rate.sleep();
}
```

### 3.2. Topic

Un **topic** es un canal de comunicación. Los nodos pueden publicar mensajes en un topic o suscribirse para recibirlos.

Ejemplo:

- `/cmd_vel`: recibe comandos de velocidad.
- `/scan`: publica lecturas del láser.
- `/odom`: publica odometría.
- `/map`: publica el mapa de ocupación.
- `/tf`: publica transformaciones entre frames.

Comandos:

```bash
rostopic list
rostopic info /topic
rostopic echo /topic
rostopic echo /topic -n1
rostopic pub /topic tipo_de_mensaje contenido
```

### 3.3. Publisher

Un **publisher** escribe mensajes en un topic.

Ejemplo de publisher a `/counter`:

```cpp
#include <ros/ros.h>
#include <std_msgs/Int32.h>

int main(int argc, char** argv) {
  ros::init(argc, argv, "topic_publisher");
  ros::NodeHandle nh;
  ros::Publisher pub = nh.advertise<std_msgs::Int32>("counter", 1000);

  ros::Rate loop_rate(2);
  int count = 0;

  while (ros::ok()) {
    std_msgs::Int32 msg;
    msg.data = count;
    pub.publish(msg);
    count++;
    ros::spinOnce();
    loop_rate.sleep();
  }
}
```

### 3.4. Subscriber

Un **subscriber** lee mensajes de un topic y ejecuta una función callback cuando llega un mensaje.

Ejemplo:

```cpp
#include <ros/ros.h>
#include <std_msgs/Int32.h>

void counterCallback(const std_msgs::Int32::ConstPtr& msg) {
  ROS_INFO("%d", msg->data);
}

int main(int argc, char** argv) {
  ros::init(argc, argv, "topic_subscriber");
  ros::NodeHandle nh;
  ros::Subscriber sub = nh.subscribe("counter", 1000, counterCallback);
  ros::spin();
  return 0;
}
```

### 3.5. Mensajes ROS

Los mensajes definen la estructura de los datos que viajan por los topics.

Comandos:

```bash
rosmsg show std_msgs/Int32
rosmsg show geometry_msgs/Twist
rosmsg show sensor_msgs/LaserScan
rosmsg show nav_msgs/Odometry
rosmsg show nav_msgs/OccupancyGrid
rosmsg show tf2_msgs/TFMessage
```

Mensajes clave:

| Mensaje | Uso |
|---|---|
| `std_msgs/Int32` | Entero simple, usado en ejemplos de contador |
| `geometry_msgs/Twist` | Velocidad lineal y angular del robot |
| `sensor_msgs/LaserScan` | Lecturas del LIDAR |
| `nav_msgs/Odometry` | Odometría del robot |
| `nav_msgs/OccupancyGrid` | Mapa de ocupación |
| `tf2_msgs/TFMessage` | Transformaciones entre frames |

### 3.6. Parameter Server

ROS tiene un servidor de parámetros donde se guardan valores de configuración compartidos por nodos.

Comandos:

```bash
rosparam list
rosparam get nombre_parametro
rosparam set nombre_parametro valor
```

Los launch files y los nodos de navegación cargan muchos parámetros en este servidor.

### 3.7. `roscore`

`roscore` inicia los servicios básicos de ROS. Sin él, normalmente los nodos no pueden descubrirse ni comunicarse.

```bash
roscore
```

---

## 4. Estructura de paquetes ROS

Un paquete ROS agrupa código, configuración, launch files, mensajes y dependencias.

Estructura típica:

```text
paquete_ros/
├── CMakeLists.txt
├── package.xml
├── launch/
│   └── archivo.launch
├── src/
│   └── nodo.cpp
├── param/
│   └── parametros.yaml
└── maps/
    ├── mapa.yaml
    └── mapa.pgm
```

### 4.1. Crear un paquete

Desde `~/catkin_ws/src`:

```bash
catkin_create_pkg my_package roscpp
```

Ejemplos de prácticas:

```bash
catkin_create_pkg topic_publisher_pkg_vostrenom roscpp std_msgs
catkin_create_pkg topic_subscriber_vostrenom_pkg roscpp std_msgs
catkin_create_pkg t3_navigation_elvostresnom roscpp
```

### 4.2. Buscar paquetes

```bash
rospack list
rospack list | grep my_package
roscd my_package
rospack profile
```

`rospack profile` sirve para regenerar la caché si ROS no detecta un paquete recién creado.

### 4.3. Compilar

Desde `~/catkin_ws`:

```bash
catkin_make
source devel/setup.bash
```

Compilar solo un paquete y sus dependencias:

```bash
catkin_make --only-pkg-with-deps nombre_paquete
```

Si un ejecutable no tiene permisos:

```bash
chmod +x nombre_del_archivo.cpp
```

En C++, normalmente el ejecutable se declara en `CMakeLists.txt`:

```cmake
add_executable(first_program src/first_program.cpp)
add_dependencies(first_program ${first_program_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(first_program ${catkin_LIBRARIES})
```

---

## 5. Launch files

Un archivo `.launch` permite iniciar varios nodos y cargar parámetros de una vez.

Comando general:

```bash
roslaunch nombre_paquete archivo.launch
```

Ejemplo de launch de teleoperación TurtleBot3:

```xml
<launch>
  <arg name="model" default="$(env TURTLEBOT3_MODEL)" doc="model type [burger, waffle, waffle_pi]"/>
  <param name="model" value="$(arg model)"/>
  <node pkg="turtlebot3_teleop"
        type="turtlebot3_teleop_key"
        name="turtlebot3_teleop_keyboard"
        output="screen">
  </node>
</launch>
```

Ejemplo de launch de un nodo propio:

```xml
<launch>
  <node pkg="my_package"
        type="first_program"
        name="Remembering_ROS"
        output="screen">
  </node>
</launch>
```

---

## 6. TurtleBot3 y simulación

El curso usa **TurtleBot3**, una plataforma móvil modular, compacta y personalizable. Está impulsada por Open Robotics, ROBOTIS y otros socios. Se usa porque es barata, educativa, compatible con ROS y adecuada para SLAM/navegación.

Modelos mencionados:

- `burger`
- `waffle`
- `waffle_pi`

Configurar modelo:

```bash
export TURTLEBOT3_MODEL=burger
```

También puede añadirse a `.bashrc`:

```bash
echo -e '\nexport TURTLEBOT3_MODEL=burger' >> ~/.bashrc
source ~/.bashrc
```

### 6.1. Instalar simulaciones TurtleBot3

```bash
cd ~/catkin_ws/src/
git clone -b noetic-devel https://github.com/ROBOTIS-GIT/turtlebot3_simulations.git
cd ~/catkin_ws
catkin_make
```

### 6.2. Mundos de Gazebo

```bash
roslaunch turtlebot3_gazebo turtlebot3_empty_world.launch
roslaunch turtlebot3_gazebo turtlebot3_world.launch
roslaunch turtlebot3_gazebo turtlebot3_house.launch
```

### 6.3. RViz para TurtleBot3

```bash
roslaunch turtlebot3_gazebo turtlebot3_gazebo_rviz.launch
rosrun rviz rviz
rosrun rviz rviz -d `rospack find turtlebot3_navigation`/rviz/turtlebot3_nav.rviz
rosrun rviz rviz -d `rospack find turtlebot3_slam`/rviz/turtlebot3_slam.rviz
```

En RViz se usan herramientas como:

- **LaserScan**: visualizar `/scan`.
- **Map**: visualizar `/map`.
- **RobotModel**: visualizar el modelo del robot.
- **TF**: visualizar frames y transformaciones.
- **PoseArray**: visualizar partículas de AMCL.
- **2D Pose Estimate**: indicar la posición inicial aproximada del robot.
- **2D Nav Goal**: enviar una meta al robot.

---

## 7. Control del robot

### 7.1. Topic `/cmd_vel`

El robot se mueve publicando mensajes en `/cmd_vel`.

```bash
rostopic info /cmd_vel
rosmsg show geometry_msgs/Twist
```

El mensaje `geometry_msgs/Twist` contiene:

```text
geometry_msgs/Vector3 linear
  float64 x
  float64 y
  float64 z
geometry_msgs/Vector3 angular
  float64 x
  float64 y
  float64 z
```

En robots de conducción diferencial como TurtleBot3 solo importan normalmente:

- `linear.x`: avance/retroceso.
- `angular.z`: giro.

Los demás campos suelen dejarse a 0.

### 7.2. Publicar velocidades desde terminal

Generar estructura automáticamente:

```bash
rostopic pub /cmd_vel [TAB][TAB]
```

Ejemplo conceptual:

```bash
rostopic pub /cmd_vel geometry_msgs/Twist "linear:
  x: 0.5
  y: 0.0
  z: 0.0
angular:
  x: 0.0
  y: 0.0
  z: 0.5"
```

Esto provoca un movimiento circular.

Para parar:

```bash
rostopic pub /cmd_vel geometry_msgs/Twist "linear:
  x: 0.0
  y: 0.0
  z: 0.0
angular:
  x: 0.0
  y: 0.0
  z: 0.0"
```

### 7.3. Teleoperación con teclado

Comando TurtleBot3:

```bash
roslaunch turtlebot3_teleop turtlebot3_teleop_key.launch
```

En algunas diapositivas también aparece:

```bash
roslaunch turtlebot_teleop keyboard_teleop.launch
```

Controles:

| Tecla | Acción |
|---|---|
| `u i o` / `j k l` / `m , .` | Movimiento direccional |
| `q` / `z` | Aumentar/disminuir velocidades máximas |
| `w` / `x` | Aumentar/disminuir velocidad lineal |
| `e` / `c` | Aumentar/disminuir velocidad angular |
| `space` o `k` | Parada forzada |
| Cualquier otra | Parada suave |
| `Ctrl-C` | Salir |

---

## 8. Sensores principales

### 8.1. LIDAR y `/scan`

El TurtleBot3 Burger usado en prácticas tiene un LIDAR de 360 grados.

Comandos:

```bash
rostopic info /scan
rostopic echo /scan -n1
rosmsg show sensor_msgs/LaserScan
```

El mensaje `sensor_msgs/LaserScan` contiene:

| Campo | Significado |
|---|---|
| `angle_min` | Ángulo inicial del barrido |
| `angle_max` | Ángulo final del barrido |
| `angle_increment` | Separación angular entre rayos |
| `range_min` | Distancia mínima válida |
| `range_max` | Distancia máxima válida |
| `ranges[]` | Array de distancias medidas |
| `intensities[]` | Intensidad de retorno, si está disponible |

La parte más importante para las prácticas es `ranges[]`.

Interpretación usada en clase:

- El láser cubre 360 grados.
- Los valores del principio del array representan la zona frontal.
- `ranges[0]` o cerca de `ranges[360]` se usa como referencia frontal.
- `ranges[180]` se interpreta como la parte trasera.
- `inf` significa que no se detecta obstáculo dentro del rango.
- Valores finitos menores que el máximo indican obstáculos.
- En las prácticas se menciona un rango aproximado de 3.5 m.

### 8.2. Odometría y `/odom`

La odometría indica el movimiento estimado del robot a partir de encoders, IMU, cámaras u otras fuentes.

Comandos:

```bash
rostopic info /odom
rostopic echo /odom
rosmsg show nav_msgs/Odometry
```

La odometría es necesaria para:

- SLAM/gmapping.
- AMCL.
- Stack de navegación.
- Estimar la posición relativa del robot desde el inicio.

### 8.3. Cámara e `/images`

El proyecto final exige usar imágenes. En el Waffle Pi se arranca la cámara con:

```bash
roslaunch turtlebot3_bringup turtlebot3_rpicamera.launch
```

Las imágenes pueden servir para:

- Seguir personas.
- Detectar objetos.
- Seguir líneas/flechas.
- Reconocer señales visuales.
- Construir mapas semánticos.

---

## 9. SLAM y mapeo

SLAM significa **Simultaneous Localization and Mapping**. Es el problema de construir un mapa de un entorno desconocido mientras se estima simultáneamente la posición del robot dentro de ese mapa.

En el curso se usa principalmente:

- Paquete: `gmapping`
- Nodo: `slam_gmapping`

### 9.1. `slam_gmapping`

`slam_gmapping` crea un mapa 2D usando:

- Datos del LIDAR.
- Transformaciones `/tf`.
- Odometría.
- Movimiento del robot.

Publica el mapa en:

```text
/map
```

Tipo de mensaje:

```text
nav_msgs/OccupancyGrid
```

En `OccupancyGrid`:

| Valor | Significado |
|---:|---|
| `0` | Libre |
| `100` | Ocupado |
| `-1` | Desconocido |

### 9.2. Flujo para crear mapa

Terminales típicas:

```bash
roscore
roslaunch turtlebot3_gazebo turtlebot3_world.launch
roslaunch turtlebot3_slam turtlebot3_slam.launch slam_methods:=gmapping
roslaunch turtlebot3_teleop turtlebot3_teleop_key.launch
rosrun rviz rviz -d `rospack find turtlebot3_slam`/rviz/turtlebot3_slam.rviz
```

Se mueve el robot por el entorno hasta cubrirlo. En RViz se visualiza:

- LaserScan.
- Map.
- RobotModel.

### 9.3. Launch propio para gmapping

En la práctica 3 se propone crear un paquete de navegación, por ejemplo `t3_navigation_elvostresnom`, con carpetas:

```text
launch/
param/
maps/
```

Ejemplo de `start_mapping.launch`:

```xml
<launch>
  <arg name="model" default="$(env TURTLEBOT3_MODEL)" doc="model type [burger, waffle, waffle_pi]"/>
  <include file="$(find turtlebot3_bringup)/launch/turtlebot3_remote.launch" />

  <node pkg="gmapping"
        type="slam_gmapping"
        name="turtlebot3_slam_gmapping"
        output="screen">
    <param name="base_frame" value="base_footprint"/>
    <param name="odom_frame" value="odom"/>
    <param name="map_update_interval" value="2.0"/>
    <param name="maxUrange" value="4.0"/>
    <param name="minimumScore" value="100"/>
    <param name="linearUpdate" value="0.2"/>
    <param name="angularUpdate" value="0.2"/>
    <param name="temporalUpdate" value="0.5"/>
    <param name="delta" value="0.05"/>
    <param name="lskip" value="0"/>
    <param name="particles" value="120"/>
    <param name="sigma" value="0.05"/>
    <param name="kernelSize" value="1"/>
    <param name="lstep" value="0.05"/>
    <param name="astep" value="0.05"/>
    <param name="iterations" value="5"/>
    <param name="lsigma" value="0.075"/>
    <param name="ogain" value="3.0"/>
    <param name="srr" value="0.01"/>
    <param name="srt" value="0.02"/>
    <param name="str" value="0.01"/>
    <param name="stt" value="0.02"/>
    <param name="resampleThreshold" value="0.5"/>
    <param name="xmin" value="-10.0"/>
    <param name="ymin" value="-10.0"/>
    <param name="xmax" value="10.0"/>
    <param name="ymax" value="10.0"/>
    <param name="llsamplerange" value="0.01"/>
    <param name="llsamplestep" value="0.01"/>
    <param name="lasamplerange" value="0.005"/>
    <param name="lasamplestep" value="0.005"/>
  </node>
</launch>
```

Parámetros importantes:

| Parámetro | Uso |
|---|---|
| `base_frame` | Frame asociado a la base móvil |
| `odom_frame` | Frame de odometría |
| `map_frame` | Frame del mapa, por defecto `map` |
| `map_update_interval` | Tiempo entre actualizaciones del mapa |
| `maxUrange` | Distancia máxima del láser usada para crear mapa |
| `throttle_scans` | Reduce consumo de recursos descartando lecturas |
| `particles` | Número de partículas en gmapping |
| `linearUpdate` | Movimiento lineal necesario para actualizar |
| `angularUpdate` | Giro necesario para actualizar |

Un `maxUrange` mayor puede crear mapas más rápido y reducir pérdidas de localización, pero consume más recursos.

---

## 10. Guardar y servir mapas

### 10.1. `map_saver`

El paquete `map_server` proporciona `map_saver`, que guarda el mapa actual.

```bash
rosrun map_server map_saver -f name_of_map
```

Genera:

```text
name_of_map.pgm
name_of_map.yaml
```

Ejemplo recomendado:

```bash
roscd t3_navigation_elvostrenom
mkdir maps
cd maps
rosrun map_server map_saver -f my_map
```

### 10.2. Archivo YAML del mapa

Ejemplo:

```yaml
image: my_map.pgm
resolution: 0.050000
origin: [-10.000000, -10.000000, 0.000000]
negate: 0
occupied_thresh: 0.65
free_thresh: 0.196
```

Campos:

| Campo | Significado |
|---|---|
| `image` | Imagen del mapa |
| `resolution` | Metros por píxel |
| `origin` | Coordenadas del píxel inferior izquierdo y rotación |
| `occupied_thresh` | Umbral para considerar ocupado |
| `free_thresh` | Umbral para considerar libre |
| `negate` | Invierte blanco/negro si vale 1 |

### 10.3. Archivo PGM

El `.pgm` es una imagen en escala de grises:

- Blanco: zona libre.
- Negro: zona ocupada.
- Gris/intermedio: zona desconocida.

Los mapas son 2D y estáticos. Si el entorno cambia después de mapear, el mapa ya no representa exactamente el mundo real.

### 10.4. `map_server`

El nodo `map_server` lee un mapa guardado y lo proporciona a otros nodos.

```bash
rosrun map_server map_server map_file.yaml
```

Ejemplo launch:

```xml
<launch>
  <node pkg="map_server"
        type="map_server"
        name="map_server"
        output="screen"
        args="/home/user/catkin_ws/src/my_map.yaml">
  </node>
</launch>
```

Servicios/topics importantes:

| Interfaz | Tipo | Uso |
|---|---|---|
| `static_map` | `nav_msgs/GetMap` | Servicio para obtener el mapa |
| `/map_metadata` | `nav_msgs/MapMetaData` | Metadatos del mapa |
| `/map` | `nav_msgs/OccupancyGrid` | Mapa de ocupación |

`/map` y `/map_metadata` son **latched topics**: guardan el último mensaje para futuros suscriptores.

---

## 11. Transformaciones y TF

Para que el robot entienda dónde están sus sensores y cómo se relacionan sus frames, se usan transformaciones.

Una transformación indica posición y orientación entre dos sistemas de coordenadas.

Ejemplos de frames:

| Frame | Significado |
|---|---|
| `map` | Sistema global del mapa |
| `odom` | Sistema de odometría |
| `base_link` | Base del robot |
| `base_footprint` | Proyección de la base del robot en el suelo |
| `base_scan` / `base_laser` | Frame del LIDAR |

### 11.1. Por qué son importantes

Si el láser detecta un obstáculo a 3 cm delante del sensor, el robot necesita saber dónde está ese obstáculo respecto al centro del robot. Para eso necesita la transformación entre el frame del láser y la base.

Para `slam_gmapping` se necesitan dos transformaciones:

1. Láser -> `base_link` o `base_footprint`.
2. `base_link`/`base_footprint` -> `odom`.

Para AMCL:

- Debe existir un camino TF desde el frame del láser hasta `odom`.
- AMCL busca la transformación entre el láser y la base y la considera fija.

### 11.2. Topic `/tf`

```bash
rostopic info /tf
rostopic echo /tf -n 10
rosmsg show tf2_msgs/TFMessage
rosrun rqt_tf_tree rqt_tf_tree
```

Tipo:

```text
tf2_msgs/TFMessage
```

Contiene:

```text
geometry_msgs/TransformStamped[] transforms
```

### 11.3. Publicar una transformación estática

```bash
rosrun tf static_transform_publisher x y z yaw pitch roll frame_id child_frame_id period_in_ms
```

Ejemplo:

```bash
rosrun tf static_transform_publisher 0 0 0 0 0 0 map odom 100
```

Ejemplo launch:

```xml
<launch>
  <node pkg="tf"
        type="static_transform_publisher"
        name="name_of_node"
        args="x y z yaw pitch roll frame_id child_frame_id period_in_ms">
  </node>
</launch>
```

Normalmente TurtleBot3 ya trae estas transformaciones configuradas.

---

## 12. Localización con AMCL

Después de crear un mapa, el robot debe localizarse dentro de él. Para ello se usa:

- Paquete: `amcl`
- Nodo: `amcl`
- Algoritmo: **Adaptive Monte Carlo Localization**

AMCL usa un filtro de partículas. Cada partícula representa una hipótesis de pose del robot. Cuando el robot se mueve y observa el entorno con el LIDAR, descarta partículas incompatibles y refuerza las más probables. Al moverse, la nube de partículas suele concentrarse alrededor de la pose real.

### 12.1. Topics de AMCL

| Topic | Uso |
|---|---|
| `/initialpose` | Pose inicial publicada desde RViz con 2D Pose Estimate |
| `/scan` | Lecturas del láser |
| `/map` | Mapa servido por `map_server` |
| `/tf` | Transformaciones |
| `/amcl_pose` | Pose estimada |
| `/particlecloud` | Nube de partículas |

### 12.2. Visualización en RViz

Para ver localización:

- Añadir `LaserScan`.
- Añadir `Map`.
- Añadir `PoseArray`.
- Topic de PoseArray: normalmente `/particlecloud`.
- Fixed Frame: `map`.
- Opcional: añadir `RobotModel` y `TF`.

Comandos:

```bash
roscore
roslaunch turtlebot3_gazebo turtlebot3_world.launch
roslaunch turtlebot3_navigation turtlebot3_navigation.launch
rosrun rviz rviz
roslaunch turtlebot3_teleop turtlebot3_teleop_key.launch
```

### 12.3. Launch propio de localización

Ejemplo de `start_localization.launch`:

```xml
<launch>
  <include file="$(find turtlebot3_bringup)/launch/turtlebot3_remote.launch" />

  <arg name="map_file" default="$(find t3_navigation)/maps/my_map.yaml"/>
  <node name="map_server" pkg="map_server" type="map_server" args="$(arg map_file)" />

  <arg name="scan_topic" default="scan"/>
  <arg name="initial_pose_x" default="0.0"/>
  <arg name="initial_pose_y" default="0.0"/>
  <arg name="initial_pose_a" default="0.0"/>

  <node pkg="amcl" type="amcl" name="amcl">
    <param name="min_particles" value="500"/>
    <param name="max_particles" value="3000"/>
    <param name="kld_err" value="0.02"/>
    <param name="update_min_d" value="0.20"/>
    <param name="update_min_a" value="0.20"/>
    <param name="resample_interval" value="1"/>
    <param name="transform_tolerance" value="0.5"/>
    <param name="recovery_alpha_slow" value="0.00"/>
    <param name="recovery_alpha_fast" value="0.00"/>
    <param name="initial_pose_x" value="$(arg initial_pose_x)"/>
    <param name="initial_pose_y" value="$(arg initial_pose_y)"/>
    <param name="initial_pose_a" value="$(arg initial_pose_a)"/>
    <param name="gui_publish_rate" value="50.0"/>
    <remap from="scan" to="$(arg scan_topic)"/>
    <param name="laser_max_range" value="3.5"/>
    <param name="laser_max_beams" value="180"/>
    <param name="laser_z_hit" value="0.5"/>
    <param name="laser_z_short" value="0.05"/>
    <param name="laser_z_max" value="0.05"/>
    <param name="laser_z_rand" value="0.5"/>
    <param name="laser_sigma_hit" value="0.2"/>
    <param name="laser_lambda_short" value="0.1"/>
    <param name="laser_likelihood_max_dist" value="2.0"/>
    <param name="laser_model_type" value="likelihood_field"/>
    <param name="odom_model_type" value="diff"/>
    <param name="odom_alpha1" value="0.1"/>
    <param name="odom_alpha2" value="0.1"/>
    <param name="odom_alpha3" value="0.1"/>
    <param name="odom_alpha4" value="0.1"/>
    <param name="odom_frame_id" value="odom"/>
    <param name="base_frame_id" value="base_footprint"/>
  </node>
</launch>
```

### 12.4. Parámetros importantes de AMCL

Parámetros generales:

| Parámetro | Significado |
|---|---|
| `odom_model_type` | Modelo de odometría: `diff`, `omni`, etc. |
| `odom_frame_id` | Frame de odometría |
| `base_frame_id` | Frame de la base del robot |
| `global_frame_id` | Frame global, normalmente `map` |
| `use_map_topic` | Si AMCL obtiene el mapa por topic o servicio |

Parámetros del filtro:

| Parámetro | Significado |
|---|---|
| `min_particles` | Número mínimo de partículas |
| `max_particles` | Número máximo de partículas |
| `kld_err` | Error máximo permitido entre distribución real y estimada |
| `update_min_d` | Distancia lineal necesaria para actualizar |
| `update_min_a` | Giro angular necesario para actualizar |
| `resample_interval` | Actualizaciones necesarias antes de remuestrear |
| `transform_tolerance` | Tolerancia temporal de la transformación |
| `gui_publish_rate` | Frecuencia máxima de publicación para visualización |

Parámetros del láser:

| Parámetro | Significado |
|---|---|
| `laser_min_range` | Distancia mínima considerada |
| `laser_max_range` | Distancia máxima considerada |
| `laser_max_beams` | Número de rayos usados para actualizar el filtro |
| `laser_model_type` | Modelo probabilístico del láser |

---

## 13. Path planning y obstacle avoidance

Para planificar rutas y evitar obstáculos se usa el **ROS Navigation Stack** y especialmente el nodo `move_base`.

El sistema completo combina:

1. `map_server`: proporciona el mapa guardado.
2. `amcl`: localiza el robot en el mapa.
3. `move_base`: planifica y ejecuta trayectorias.
4. Costmaps: representan zonas ocupadas, libres e infladas.
5. Planner global: calcula ruta global.
6. Planner local: genera comandos inmediatos evitando obstáculos.

### 13.1. Launch de navegación

Ejemplo `start_navigation.launch`:

```xml
<launch>
  <arg name="model" default="burger" doc="model type [burger, waffle]"/>

  <include file="$(find t3_navigation)/launch/start_localization.launch"/>

  <arg name="cmd_vel_topic" default="/cmd_vel" />
  <arg name="odom_topic" default="odom" />

  <node pkg="move_base"
        type="move_base"
        respawn="false"
        name="move_base"
        output="screen">
    <param name="base_local_planner" value="dwa_local_planner/DWAPlannerROS" />

    <rosparam file="$(find t3_navigation)/param/costmap_common_params_$(arg model).yaml"
              command="load"
              ns="global_costmap" />
    <rosparam file="$(find t3_navigation)/param/costmap_common_params_$(arg model).yaml"
              command="load"
              ns="local_costmap" />
    <rosparam file="$(find t3_navigation)/param/local_costmap_params.yaml" command="load" />
    <rosparam file="$(find t3_navigation)/param/global_costmap_params.yaml" command="load" />
    <rosparam file="$(find t3_navigation)/param/move_base_params.yaml" command="load" />
    <rosparam file="$(find t3_navigation)/param/dwa_local_planner_params.yaml" command="load" />

    <remap from="cmd_vel" to="$(arg cmd_vel_topic)"/>
    <remap from="odom" to="$(arg odom_topic)"/>
  </node>
</launch>
```

### 13.2. Costmap común

Archivo `costmap_common_params_burger.yaml`:

```yaml
obstacle_range: 2.5
raytrace_range: 3.5
footprint: [[-0.110, -0.090], [-0.110, 0.090], [0.041, 0.090], [0.041, -0.090]]
# robot_radius: 0.105
inflation_radius: 0.15
cost_scaling_factor: 0.5
map_type: costmap
transform_tolerance: 0.2
observation_sources: scan
scan: {data_type: LaserScan, topic: scan, marking: true, clearing: true}
```

Conceptos:

| Parámetro | Uso |
|---|---|
| `obstacle_range` | Distancia a la que se consideran obstáculos |
| `raytrace_range` | Distancia usada para limpiar espacio libre |
| `footprint` | Polígono físico del robot |
| `inflation_radius` | Margen de seguridad alrededor de obstáculos |
| `observation_sources` | Sensores usados para el costmap |
| `scan` | Configuración del LIDAR como fuente |

### 13.3. Local costmap

Archivo `local_costmap_params.yaml`:

```yaml
local_costmap:
  global_frame: odom
  robot_base_frame: base_footprint
  update_frequency: 2.0
  publish_frequency: 0.5
  static_map: false
  rolling_window: true
  width: 3.5
  height: 3.5
  resolution: 0.05
  transform_tolerance: 1.0
```

El local costmap:

- Se construye con lecturas actuales del láser.
- No usa mapa estático.
- Tiene una ventana móvil alrededor del robot.
- Sirve para evitar obstáculos cercanos en tiempo real.

### 13.4. Global costmap

Archivo `global_costmap_params.yaml`:

```yaml
global_costmap:
  global_frame: map
  robot_base_frame: base_footprint
  update_frequency: 2.0
  publish_frequency: 0.1
  static_map: true
  transform_tolerance: 1.0
```

El global costmap:

- Usa el mapa estático.
- Sirve para planificar rutas globales.
- Su frame global es `map`.

### 13.5. Parámetros generales de `move_base`

Archivo `move_base_params.yaml`:

```yaml
shutdown_costmaps: false
controller_frequency: 3.0
controller_patience: 1.0
planner_frequency: 2.0
planner_patience: 1.0
oscillation_timeout: 10.0
oscillation_distance: 0.2
conservative_reset_dist: 0.10
```

Conceptos:

| Parámetro | Uso |
|---|---|
| `controller_frequency` | Frecuencia del controlador local |
| `planner_frequency` | Frecuencia del planificador global |
| `controller_patience` | Tiempo de espera del controlador |
| `planner_patience` | Tiempo de espera del planificador |
| `oscillation_timeout` | Tiempo para detectar oscilaciones |
| `oscillation_distance` | Distancia asociada a oscilación |
| `conservative_reset_dist` | Distancia para reset conservador del costmap |

### 13.6. DWA Local Planner

Archivo `dwa_local_planner_params.yaml`:

```yaml
DWAPlannerROS:
  max_vel_x: 0.18
  min_vel_x: -0.18
  max_vel_y: 0.0
  min_vel_y: 0.0

  max_trans_vel: 0.18
  min_trans_vel: 0.05

  max_rot_vel: 1.8
  min_rot_vel: 0.7

  acc_lim_x: 2.0
  acc_lim_theta: 2.0
  acc_lim_y: 0.0

  yaw_goal_tolerance: 0.15
  xy_goal_tolerance: 0.05

  sim_time: 3.5
  vx_samples: 20
  vy_samples: 0
  vtheta_samples: 40

  path_distance_bias: 32.0
  goal_distance_bias: 24.0
  occdist_scale: 0.04
  forward_point_distance: 0.325
  stop_time_buffer: 0.2
  scaling_speed: 0.25
  max_scaling_factor: 0.2

  oscillation_reset_dist: 0.05

  publish_traj_pc: true
  publish_cost_grid_pc: true
```

DWA genera trayectorias locales posibles y las puntúa según:

- Cercanía al camino global.
- Cercanía al objetivo.
- Distancia a obstáculos.
- Límites de velocidad/aceleración.
- Tolerancias de llegada.

### 13.7. Ejecutar navegación

```bash
roscore
roslaunch turtlebot3_gazebo turtlebot3_world.launch
roslaunch t3_navigation start_navigation.launch
rosrun rviz rviz -d `rospack find turtlebot3_navigation`/rviz/turtlebot3_nav.rviz
```

En RViz:

1. Usar **2D Pose Estimate** para localizar aproximadamente el robot.
2. Usar **2D Nav Goal** para mandar una meta.
3. Observar plan global, plan local, mapa, costmaps, láser y partículas.

---

## 14. Obstáculos dinámicos y pruebas en Gazebo

El sistema de evitación de obstáculos usa costmaps locales actualizados con sensores en tiempo real. Aunque el mapa estático no tenga un obstáculo nuevo, el LIDAR puede detectarlo y el local costmap puede modificar la trayectoria.

### 14.1. Añadir un obstáculo manual

Ejemplo `Object.urdf`:

```xml
<robot name="simple_box">
  <link name="my_box">
    <inertial>
      <origin xyz="2 0 0" />
      <mass value="1.0" />
      <inertia ixx="1.0" ixy="0.0" ixz="0.0" iyy="100.0" iyz="0.0" izz="1.0" />
    </inertial>
    <visual>
      <origin xyz="2 0 1"/>
      <geometry>
        <box size="1 1 2" />
      </geometry>
    </visual>
    <collision>
      <origin xyz="2 0 1"/>
      <geometry>
        <box size="1 1 2" />
      </geometry>
    </collision>
  </link>
  <gazebo reference="my_box">
    <material>Gazebo/Blue</material>
  </gazebo>
</robot>
```

Spawn:

```bash
rosrun gazebo_ros spawn_model -file ~/catkin_ws/src/urdf/object.urdf -urdf -x 0 -y 0 -z 1 -model my_object
```

Eliminar:

```bash
rosservice call /gazebo/delete_model "model_name: 'my_object'"
```

Nota importante de clase: el obstáculo puede no aparecer dibujado en el mapa estático de RViz, pero el LIDAR sí lo detecta y el costmap local debe reaccionar.

---

## 15. URDF y configuración del robot

URDF significa **Unified Robot Description Format**. Es un formato XML para describir:

- Partes del robot.
- Dimensiones.
- Cinemática.
- Dinámica.
- Sensores.
- Colisiones.
- Visualización.

Ejemplo de definición de láser:

```xml
<joint name="laser_sensor_joint" type="fixed">
  <origin xyz="0.0 0.0 0.435" rpy="0 0 0"/>
  <parent link="base_link"/>
  <child link="laser_sensor_link"/>
</joint>

<link name="laser_sensor_link">
  <inertial>
    <mass value="1e-5"/>
    <origin xyz="0 0 0" rpy="0 0 0"/>
    <inertia ixx="1e-6" ixy="0" ixz="0" iyy="1e-6" iyz="0" izz="1e-6"/>
  </inertial>
  <collision>
    <origin xyz="0 0 0" rpy="0 0 0"/>
    <geometry>
      <box size="0.1 0.1 0.1"/>
    </geometry>
  </collision>
  <visual>
    <origin xyz="0 0 0" rpy="0 0 0"/>
    <geometry>
      <mesh filename="package://hokuyo/meshes/hokuyo.dae"/>
    </geometry>
  </visual>
</link>
```

En el curso no se trabaja profundamente con URDF, pero se introduce porque explica de dónde salen muchas transformaciones y modelos visuales.

---

## 16. Stack de navegación ROS

El **Navigation Stack** es un conjunto de nodos y algoritmos que permite mover un robot autónomamente desde una posición A hasta una posición B evitando obstáculos.

Entradas principales:

| Entrada | Tipo/Topic |
|---|---|
| Pose actual/localización | AMCL, `/amcl_pose`, `/tf` |
| Meta deseada | RViz `2D Nav Goal` |
| Odometría | `/odom`, `nav_msgs/Odometry` |
| Sensor de obstáculos | `/scan`, `sensor_msgs/LaserScan` |
| Transformaciones | `/tf`, `tf2_msgs/TFMessage` |
| Mapa | `/map`, `nav_msgs/OccupancyGrid` |

Salida principal:

| Salida | Tipo |
|---|---|
| `/cmd_vel` | `geometry_msgs/Twist` |

Requisitos de hardware:

- Robot diferencial u holonómico.
- Controlable mediante velocidades lineales/angular.
- Sensor planar tipo LIDAR.
- Buena odometría.
- Transformaciones correctas.
- Forma física simple, preferiblemente circular o rectangular.

### 16.1. Nodo `move_base`

`move_base` es el nodo central del stack de navegación. Integra:

- Global planner.
- Local planner.
- `rotate_recovery`.
- `clear_costmap_recovery`.
- `costmap_2d`.
- Map server.
- AMCL.
- Gmapping.

Su función es mover el robot desde su pose actual hasta una pose objetivo, generando comandos `/cmd_vel`.

---

## 17. Tabla de nodos importantes

| Nodo | Paquete | Función |
|---|---|---|
| `roscore` | ROS core | Arranca servicios centrales |
| `turtlebot3_robot.launch` | `turtlebot3_bringup` | Arranca robot real |
| `turtlebot3_remote.launch` | `turtlebot3_bringup` | Configuración remota TurtleBot3 |
| `turtlebot3_rpicamera.launch` | `turtlebot3_bringup` | Arranca cámara en Waffle Pi |
| `turtlebot3_empty_world.launch` | `turtlebot3_gazebo` | Simulación mundo vacío |
| `turtlebot3_world.launch` | `turtlebot3_gazebo` | Simulación mundo TurtleBot3 |
| `turtlebot3_house.launch` | `turtlebot3_gazebo` | Simulación casa |
| `turtlebot3_teleop_key` | `turtlebot3_teleop` | Teleoperación por teclado |
| `slam_gmapping` | `gmapping` | SLAM y construcción de mapa |
| `map_saver` | `map_server` | Guardar mapa |
| `map_server` | `map_server` | Servir mapa guardado |
| `amcl` | `amcl` | Localización por partículas |
| `move_base` | `move_base` | Navegación, planificación y evitación |
| `rviz` | `rviz` | Visualización |
| `spawn_model` | `gazebo_ros` | Insertar objetos en Gazebo |
| `static_transform_publisher` | `tf` | Publicar transformaciones fijas |
| `rqt_tf_tree` | `rqt_tf_tree` | Ver árbol TF |

---

## 18. Tabla de topics importantes

| Topic | Mensaje | Uso |
|---|---|---|
| `/cmd_vel` | `geometry_msgs/Twist` | Velocidades del robot |
| `/scan` | `sensor_msgs/LaserScan` | Lecturas LIDAR |
| `/odom` | `nav_msgs/Odometry` | Odometría |
| `/tf` | `tf2_msgs/TFMessage` | Transformaciones dinámicas |
| `/tf_static` | `tf2_msgs/TFMessage` | Transformaciones estáticas |
| `/map` | `nav_msgs/OccupancyGrid` | Mapa de ocupación |
| `/map_metadata` | `nav_msgs/MapMetaData` | Metadatos del mapa |
| `/initialpose` | `geometry_msgs/PoseWithCovarianceStamped` | Pose inicial para AMCL |
| `/amcl_pose` | `geometry_msgs/PoseWithCovarianceStamped` | Pose estimada |
| `/particlecloud` | `geometry_msgs/PoseArray` | Partículas de AMCL |
| `/move_base/goal` | `move_base_msgs/MoveBaseActionGoal` | Objetivo de navegación |
| `/images` | Imagen/cámara | Procesamiento visual del proyecto final |

---

## 19. Chuleta de comandos

### ROS básico

```bash
roscore
rosnode list
rosnode info /nombre_nodo
rostopic list
rostopic info /topic
rostopic echo /topic
rostopic echo /topic -n1
rosmsg show tipo/Mensaje
rosparam list
rosparam get parametro
rosparam set parametro valor
```

### Paquetes

```bash
cd ~/catkin_ws/src
catkin_create_pkg nombre_paquete roscpp std_msgs geometry_msgs sensor_msgs
cd ~/catkin_ws
catkin_make
source devel/setup.bash
rospack list
rospack list | grep nombre_paquete
roscd nombre_paquete
rospack profile
```

### TurtleBot3

```bash
export TURTLEBOT3_MODEL=burger
export TURTLEBOT3_MODEL=waffle_pi
roslaunch turtlebot3_gazebo turtlebot3_empty_world.launch
roslaunch turtlebot3_gazebo turtlebot3_world.launch
roslaunch turtlebot3_gazebo turtlebot3_house.launch
roslaunch turtlebot3_teleop turtlebot3_teleop_key.launch
```

### RViz

```bash
rosrun rviz rviz
roslaunch turtlebot3_gazebo turtlebot3_gazebo_rviz.launch
rosrun rviz rviz -d `rospack find turtlebot3_navigation`/rviz/turtlebot3_nav.rviz
rosrun rviz rviz -d `rospack find turtlebot3_slam`/rviz/turtlebot3_slam.rviz
```

### Mapeo

```bash
roslaunch turtlebot3_slam turtlebot3_slam.launch slam_methods:=gmapping
rosrun map_server map_saver -f my_map
```

### Servir mapa

```bash
rosrun map_server map_server my_map.yaml
```

### Navegación

```bash
roslaunch turtlebot3_navigation turtlebot3_navigation.launch
roslaunch t3_navigation start_mapping.launch
roslaunch t3_navigation start_localization.launch
roslaunch t3_navigation start_navigation.launch
```

### TF

```bash
rostopic info /tf
rostopic echo /tf -n 10
rostopic echo /tf_static -n1
rosrun rqt_tf_tree rqt_tf_tree
rosrun tf static_transform_publisher 0 0 0 0 0 0 map odom 100
```

### Gazebo

```bash
rosrun gazebo_ros spawn_model -file ~/catkin_ws/src/urdf/object.urdf -urdf -x 0 -y 0 -z 1 -model my_object
rosservice call /gazebo/delete_model "model_name: 'my_object'"
```

---

## 20. Lo que hemos hecho en este repositorio

En el repositorio hay una solución de la práctica 2:

```text
Context/solucio_P2 RM- Topics de RO/
├── CMakeLists.txt
├── package.xml
├── launch/
│   └── scan_subscriber_pau_joel.launch
└── src/
    └── scan_subscriber_pau_joel.cpp
```

### 20.1. Objetivo de esa práctica

Crear un nodo que:

1. Se suscribe a `/scan`.
2. Lee el LIDAR.
3. Detecta obstáculos delante.
4. Publica comandos de velocidad en `/cmd_vel`.
5. Avanza si el camino está libre.
6. Gira si detecta obstáculo.

Esto corresponde directamente al entregable de P2: navegación reactiva básica usando LIDAR y `/cmd_vel`.

### 20.2. Nodo implementado

Archivo:

```text
Context/solucio_P2 RM- Topics de RO/src/scan_subscriber_pau_joel.cpp
```

Incluye:

```cpp
#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include <cmath>
```

Variables globales/parámetros:

```cpp
ros::Publisher pub;

int consecutiveNeeded = 3;
float vel_robot = 0.35;
float turn_speed = 0.3;
float min_distance = 1.0;
```

Lógica:

- Revisa las lecturas frontales del LIDAR.
- Comprueba índices cercanos al frente:
  - `0..14`
  - `345..359`
- Ignora valores no finitos con `std::isfinite`.
- Cuenta lecturas consecutivas por debajo de `min_distance`.
- Si hay al menos `consecutiveNeeded` lecturas peligrosas, considera que hay obstáculo.

Comportamiento:

```text
Si obstáculo:
  linear.x = 0.0
  angular.z = turn_speed
  mensaje: "Obstaculo detectado: girando"

Si camino libre:
  linear.x = vel_robot
  angular.z = 0.0
  mensaje: "Camino libre: avanzando"
```

Publicación:

```cpp
pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 1000);
```

Suscripción:

```cpp
ros::Subscriber sub = nh.subscribe("/scan", 100, scan_function);
```

Ejecución:

```cpp
ros::spin();
```

### 20.3. Launch del nodo

Archivo:

```text
Context/solucio_P2 RM- Topics de RO/launch/scan_subscriber_pau_joel.launch
```

Contenido:

```xml
<launch>
  <node pkg="scan_subscriber_pau_joel"
        type="scan_subscriber_pau_joel"
        name="scan_subscriber_pau_joel"
        output="screen">
  </node>
</launch>
```

### 20.4. `CMakeLists.txt`

El paquete declara:

- `roscpp`
- `std_msgs`
- `geometry_msgs`
- `sensor_msgs`

Y crea el ejecutable:

```cmake
add_executable(scan_subscriber_pau_joel src/scan_subscriber_pau_joel.cpp)
add_dependencies(scan_subscriber_pau_joel ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(scan_subscriber_pau_joel ${catkin_LIBRARIES})
```

### 20.5. Observación sobre `package.xml`

El `CMakeLists.txt` incluye `geometry_msgs` y `sensor_msgs`, pero el `package.xml` local solo declara `roscpp` y `std_msgs`.

Para que el paquete quede más correcto y portable, convendría añadir:

```xml
<build_depend>geometry_msgs</build_depend>
<build_depend>sensor_msgs</build_depend>
<build_export_depend>geometry_msgs</build_export_depend>
<build_export_depend>sensor_msgs</build_export_depend>
<exec_depend>geometry_msgs</exec_depend>
<exec_depend>sensor_msgs</exec_depend>
```

---

## 21. Relación entre prácticas y proyecto final

La práctica 1 da la base de ROS:

- Paquetes.
- Launch files.
- Nodos.
- `roscore`.
- Parameter Server.
- Simulación TurtleBot3.
- RViz.
- SLAM básico.

La práctica 2 añade comunicación por topics:

- Publishers.
- Subscribers.
- Mensajes.
- `/cmd_vel`.
- `/scan`.
- Control reactivo.

La práctica 3 construye navegación completa:

- Crear mapas.
- Guardar mapas.
- AMCL.
- `move_base`.
- Costmaps.
- DWA local planner.
- Obstáculos dinámicos.
- URDF.
- Navigation Stack.

La práctica 4 profundiza en mapeo/localización:

- SLAM.
- `gmapping`.
- `map_server`.
- PGM/YAML.
- TF.
- MCL.
- AMCL.
- Parámetros de localización.

El proyecto final junta todo:

- Movimiento: `/cmd_vel`.
- Percepción láser: `/scan`.
- Percepción visual: `/images`.
- Navegación/autonomía.
- Una conducta inteligente de mayor nivel.

---

## 22. Flujo recomendado para un proyecto final sólido

1. Definir misión:
   - Seguir persona.
   - Buscar objeto.
   - Patrullar.
   - Navegar visualmente.
   - Construir mapa semántico.

2. Arrancar robot/simulación:

```bash
export TURTLEBOT3_MODEL=waffle_pi
roscore
roslaunch turtlebot3_gazebo turtlebot3_world.launch
```

3. Verificar topics:

```bash
rostopic list
rostopic info /cmd_vel
rostopic info /scan
rostopic info /odom
rostopic info /tf
```

4. Verificar sensores:

```bash
rostopic echo /scan -n1
rostopic echo /odom -n1
```

5. Implementar nodo propio:

- Subscriber de `/scan`.
- Subscriber de imagen.
- Publisher de `/cmd_vel`.
- Opcional: integración con navegación global.

6. Probar primero en Gazebo.

7. Visualizar en RViz:

- RobotModel.
- LaserScan.
- Map.
- TF.
- Costmaps.
- Path.

8. Grabar vídeos.

9. Preparar demo.

10. Explicar arquitectura:

```text
Sensores -> percepción -> decisión -> control -> /cmd_vel -> robot
```

---

## 23. Arquitectura conceptual del proyecto

```text
              +------------------+
              |      Cámara      |
              |     /images      |
              +---------+--------+
                        |
                        v
+---------+     +-------+--------+      +----------------+
| LIDAR   | --> | Nodo propio    | ---> | /cmd_vel       |
| /scan   |     | percepción +   |      | Twist          |
+---------+     | decisión       |      +-------+--------+
                +-------+--------+              |
                        |                       v
                        |               +---------------+
                        |               | Base móvil    |
                        |               | TurtleBot3    |
                        |               +---------------+
                        |
                        v
              +------------------+
              | RViz/Gazebo      |
              | depuración       |
              +------------------+
```

Si se usa navegación completa:

```text
Mapa + AMCL + TF + Odom + Scan + Goal
                 |
                 v
              move_base
                 |
                 v
              /cmd_vel
```

---

## 24. Conceptos clave que hay que dominar

| Concepto | Qué significa |
|---|---|
| ROS | Middleware para robótica |
| Nodo | Proceso que realiza una tarea |
| Topic | Canal de comunicación publish/subscribe |
| Publisher | Nodo que escribe en un topic |
| Subscriber | Nodo que lee de un topic |
| Message | Tipo de dato que circula por topics |
| Launch file | Archivo para iniciar nodos/parámetros |
| Catkin | Sistema de build usado en ROS Noetic |
| Gazebo | Simulador físico |
| RViz | Visualizador de datos ROS |
| TurtleBot3 | Plataforma móvil usada en prácticas |
| `/cmd_vel` | Topic de velocidad |
| `/scan` | Topic de LIDAR |
| `/odom` | Topic de odometría |
| `/tf` | Topic de transformaciones |
| SLAM | Construir mapa y localizarse simultáneamente |
| Gmapping | Algoritmo/nodo SLAM usado |
| OccupancyGrid | Mapa de ocupación |
| `map_saver` | Guarda mapa |
| `map_server` | Sirve mapa |
| MCL | Localización Monte Carlo |
| AMCL | Localización Monte Carlo adaptativa |
| Partículas | Hipótesis de pose del robot |
| Costmap | Mapa de costes para navegación |
| Local costmap | Costmap dinámico cercano |
| Global costmap | Costmap basado en mapa global |
| DWA | Planner local de trayectorias |
| `move_base` | Nodo principal de navegación |
| URDF | Descripción XML del robot |
| TF tree | Árbol de frames del robot |

---

## 25. Resumen final

La asignatura parte de ROS básico y avanza hasta navegación autónoma. Primero se aprende a crear paquetes y nodos, después a comunicar nodos mediante topics y mensajes, luego a controlar el TurtleBot3 mediante `/cmd_vel` y leer sensores como `/scan` y `/odom`. Con esas bases se construyen mapas usando `gmapping`, se guardan con `map_saver`, se sirven con `map_server`, se localiza el robot con `amcl` y se planifican rutas con `move_base`.

El trabajo local del repositorio ya contiene una solución reactiva de evitación de obstáculos: un nodo C++ que se suscribe al LIDAR `/scan` y publica velocidades en `/cmd_vel`. Esta solución es una base directa para el proyecto final porque implementa el ciclo fundamental de un robot móvil:

```text
leer sensores -> decidir acción -> publicar velocidad
```

Para el proyecto final hay que ampliar esa idea con una misión más completa, usando además imágenes de cámara y una arquitectura que pueda demostrarse en simulación o robot real.
