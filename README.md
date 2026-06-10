Integrantes
Aaren Asher Juárez Arellano Juárez 10442
Jesús Torres Vargas 10511

1. ¿Por qué la variable g_ledRapido debe declararse como volatile? ¿Qué ocurre si se omite?
Es para advertirle al compilador que esa variable puede cambiar en cualquier momento desde otro lado. Si no le ponemos volatile, el compilador guarda es variable en la memoria, entonces si otra tarea modifica la variable esta sigue leyendo el dato viejo y el programa no hace lo que debería.

2. ¿En qué momento exacto aparece el mensaje [IDLE] en la terminal? Describe el estado de las tareas.
Ese mensaje sale en el momento en el que todas nuestras tareas están bloqueadas. Como el procesador no puede simplemente apagarse o quedarse sin hacer nada, FreeRTOS mete automáticamente esta tarea Idle de relleno. En ese instante, las cuatro tareas que hicimos están en estado Blocked o Suspended, y solo la tarea Idle está corriendo porque tiene la prioridad más baja de todas.

3. ¿Qué diferencia existe entre vTaskDelay() y vTaskDelayUntil()? ¿Dónde usaríamos el Until?
El vTaskDelay normal te hace un retardo contando a partir de que lee la instrucción. El vTaskDelayUntil es más exacto porque genera un retardo absoluto, toma en cuenta el tiempo que tardó en ejecutarse el código y lo descuenta. 

4. ¿Por qué vTaskLedRapido tiene prioridad menor que vTaskMonitor? ¿Qué pasa si se invierten?
La tarea de monitoreo tiene que ser la prioridad porque necesitamos que el sistema reaccione a lo que hace el usuario. Si las invirtiéramos, el microcontrolador le daría preferencia a estar parpadeando el LED, entonces, si presionamos el botón, el sistema nos ignoraría hasta que termine con el LED y se sentiría todo trabado.

5. ¿Qué riesgo existe al leer una variable volatile sin protección y qué es una sección crítica?
Si una tarea está a la mitad de cambiar el valor de esa variable, y justo ahí FreeRTOS le da el turno a otra tarea para que la lea, esta segunda va a leer pura "basura" . Para que no pase eso usamos una sección crítica, que espera hasta que la tarea esta terminada.
 