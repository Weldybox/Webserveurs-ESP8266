------------------------ESP8266 & BME280------------------------

Cette wemos d1 mini embarque un serveur web qui affiche un graphique de temp�rature.

- Le code arduino permet de r�cup�rer et stocker la temp�rature dans le fichier temperature.csv.
Il permet �galement de maintenant le serveur Web stock� dans la m�moire flash SPIFFS de l'ESP8266.

- le code HTML/CSS/JS r�cup�re les donn�es stock�es dans le fichier temperature.csv et l'affiche sous forme de grapphique.
Le framework highcharts avec highstocks permet d'avoir un aper�us de haute qualit� de la temp�rature.