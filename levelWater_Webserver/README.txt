Dans ce dossier vous retrouverez:

- un code arduino pour NodeMCU
- un code HTML/CSS
- un sch�ma Fritzing

---------------------------------------------------------------------------------------------------------------------------------------

Le code permet de faire fonctionner un serveur web sur le nodeMCU. Celui-ci est con�u remplir un r�servoir d'eau avec l'aide d'une pompe.

A NOTER : le code HTML/CSS est d�j� impl�ment� dans le code Arduino.

Le code embarque un site web qui sera �crit dans la m�moire flash de l'ESP.
Le site web fonctionne avec AJAX et va permettre d'interagir avec celui-ci.

Le bouton "ON" appelle une fonction JS qui va �mettre une requ�te GET et ainsi, activer le m�canisme d'AJAX.
Cela va actionner la pompe et afficher son �tat sur le serveur dans un premier temps, puis d�sactiver la pompe et afficher son �tat sur le serveur quand le niveau d'eau sera correct.

---------------------------------------------------------------------------------------------------------------------------------------

Le code est enti�rement comment� pour que vous puissiez le comprendre au mieux.

LIEN YOUTUBE DE LA VIDEO TEST : https://www.youtube.com/watch?v=6l_FuSFpe3Y&feature=youtu.be
