# Air Quality Monitor (PM 2.5, HCHO, Temperature & Humidity)

Este proyecto implementa un monitor de calidad del aire utilizando el módulo SEN0233 de DFRobot. El código está diseñado para detectar la concentración de formaldehído (HCHO), partículas PM1.0, PM2.5 y PM10, así como la temperatura y la humedad en el ambiente.

## Descripción

El código lee datos del sensor SEN0233, que mide varios parámetros ambientales. Luego, los datos se procesan y se muestran en la consola serie. El sistema también verifica la integridad de los datos recibidos mediante un control de redundancia cíclica (CRC).

### Parámetros monitoreados:
- **Temperatura:** en grados Celsius (°C)
- **Humedad:** en porcentaje (%)
- **PM1.0:** concentración de partículas PM1.0 en microgramos por metro cúbico (µg/m³)
- **PM2.5:** concentración de partículas PM2.5 en microgramos por metro cúbico (µg/m³)
- **PM10:** concentración de partículas PM10 en microgramos por metro cúbico (µg/m³)
- **Formaldehído (HCHO):** concentración de formaldehído en miligramos por metro cúbico (mg/m³)

## Enlace de compra

Puedes adquirir el módulo SEN0233 aquí: [DFRobot SEN0233](https://www.dfrobot.com/product-1612.html)

## Requisitos de hardware

- Módulo SEN0233
- Microcontrolador compatible con Arduino
- Cables de conexión

## Cómo funciona

1. El código inicializa la comunicación serial con el módulo SEN0233 a 9600 baudios.
2. En cada ciclo del loop, se leen los datos enviados por el sensor.
3. Los datos son procesados y se realiza un CRC para verificar su integridad.
4. Si los datos son válidos, se calculan y muestran los valores de temperatura, humedad, PM1.0, PM2.5, PM10 y formaldehído en la consola serie.
5. El intervalo de actualización está configurado en 3 segundos.

## Código fuente

El código fuente se encuentra en el archivo `SEN0233.ino`.

## Licencia

Este proyecto está licenciado bajo la Licencia MIT. Consulta el archivo LICENSE para obtener más información.

---

Modificado por Alejandro Rebolledo para incluir la detección de formaldehído (HCHO), que según el datasheet se encuentra en un dato antes de la temperatura.
