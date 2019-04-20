Dans ce dossier vous retrouverez:

- un code arduino pour NodeMCU
- un code HTML/CSS
- un schéma Fritzing

---------------------------------------------------------------------------------------------------------------------------------------

Le code permet de faire fonctionner un serveur web sur le nodeMCU. Celui-ci est conçu remplir un réservoir d'eau avec l'aide d'une pompe.

A NOTER : le code HTML/CSS est déjà implémenté dans le code Arduino.

Le code embarque un site web qui sera écrit dans la mémoire flash de l'ESP.
Le site web fonctionne avec AJAX et va permettre d'interagir avec celui-ci.

Le bouton "ON" appelle une fonction JS qui va émettre une requête GET et ainsi, activer le mécanisme d'AJAX.
Cela va actionner la pompe et afficher son état sur le serveur dans un premier temps, puis désactiver la pompe et afficher son état sur le serveur quand le niveau d'eau sera correct.

---------------------------------------------------------------------------------------------------------------------------------------

Le code est entièrement commenté pour que vous puissiez le comprendre au mieux.

LIEN YOUTUBE DE LA VIDEO TEST : https://www.youtube.com/watch?v=6l_FuSFpe3Y&feature=youtu.be
