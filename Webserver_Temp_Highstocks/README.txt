------------------------ESP8266 & BME280------------------------

Cette wemos d1 mini embarque un serveur web qui affiche un graphique de température.

- Le code arduino permet de récupérer et stocker la température dans le fichier temperature.csv.
Il permet également de maintenant le serveur Web stocké dans la mémoire flash SPIFFS de l'ESP8266.

- le code HTML/CSS/JS récupère les données stockées dans le fichier temperature.csv et l'affiche sous forme de grapphique.
Le framework highcharts avec highstocks permet d'avoir un aperçus de haute qualité de la température.