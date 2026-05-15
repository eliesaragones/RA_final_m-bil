# RA_final_m-bil

Repositorio del proyecto final de Robotica Mobil.

## Proyecto: MWC TaxiBot

MWC TaxiBot es un taxi autonomo indoor basado en ROS para un entorno ferial inspirado en el Mobile World Congress. El robot recibe una solicitud de trayecto, se desplaza al punto de recogida, espera al usuario, navega hasta el destino y vuelve a base o queda disponible para otra peticion.

## Contenido del repositorio

- `mwc_taxibot/`: paquete ROS 1 en C++ con la implementacion del sistema.
- `docs/PROPUESTA_MWC_TAXIBOT.md`: propuesta completa del proyecto, arquitectura, nodos, topics, servicios, estados, simulacion y demo.
- `docs/RESUMEN_ASIGNATURA_PROYECTO_FINAL.md`: resumen detallado de la asignatura, conceptos, comandos y contexto de los PDFs.
- `Enunciats_labs/`: enunciados de practicas.
- `RA- ProjecteFinal-2026-RM.pdf`: enunciado del proyecto final.

## Estado actual

La primera version implementada incluye:

- Nodo principal `taxi_manager_node`.
- Libreria C++ `taxi_core` con maquina de estados.
- Servicio `/request_taxi`.
- Topic `/taxi_status`.
- Cliente de terminal `taxi_request_client`.
- Nodo `vision_perception_node` con percepcion simulada y deteccion basica por colores.
- Configuracion de zonas del recinto MWC.
- Launch files para simulacion y robot real.
- Mundo Gazebo simplificado.
- Test unitario de la logica principal.

## Como compilar

Desde un workspace Catkin:

```bash
mkdir -p ~/catkin_ws/src
cp -R mwc_taxibot ~/catkin_ws/src/
cd ~/catkin_ws
catkin_make
source devel/setup.bash
```

## Como ejecutar

Primero debe existir un robot o simulador con mapa, odometria/localizacion y servidor `move_base`.

Lanzar TaxiBot:

```bash
roslaunch mwc_taxibot mwc_taxibot_sim.launch
```

Pedir un trayecto:

```bash
rosrun mwc_taxibot taxi_request_client entrada auditorio
```

Ver estado:

```bash
rostopic echo /taxi_status
```

## Verificacion realizada antes de subir

- Compilacion local del nucleo C++ puro con `clang++`.
- Ejecucion correcta de `taxi_core_test`.
- Validacion XML de `package.xml`, launch files y mundo Gazebo.

Pendiente de validar en entorno ROS:

- `catkin_make`.
- Ejecucion real contra `move_base`.
- Ajuste de coordenadas en mapa real/simulado.
- Integracion final con robot fisico o simulador usado en clase.
