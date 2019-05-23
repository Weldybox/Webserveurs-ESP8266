

var on = true;

var red;
var green;
var blue;

var rangeDefine;
var pickColor = 0;


/*---------------------------------------------------------
Différents objets et fonction relatifs au Websocket
---------------------------------------------------------*/
var connection = new WebSocket('ws://' + location.hostname + ':81/',['arduino']);
connection.onmessage = function(event){
  console.log(event.data);
  colorPicker.color.rgbString = event.data;
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onclose = function () {
  console.log('WebSocket connection closed');
};
  
function chooseColorRange(i){
  console.log("fonction de changement de background color");
  pickColor = i;
}


window.onload=function(){

  var bouton = document.getElementById('btnMenu');
  var nav = document.getElementById('nav');
  bouton.onclick = function(e){
      if(nav.style.display=="block"){
          nav.style.display="none";
      }else{
          nav.style.display="block";
      }
  };
};


/*---------------------------------------------------------
Fonction montre les couleurs sauvegardé par l'utilisateur
---------------------------------------------------------*/
function displaySave(results){
  if(results.data[0][0] == null){
    var number = 1;
  }else{var number =0;}
  console.log(results);
  var count = 0;
  results.data[number].forEach(element => {
    var id= 'carre' + String(count+1);
    console.log(element[count]);

    var backgroundcolor = "rgb(" + element + ")";
    document.getElementById(id).style.background = backgroundcolor;
    count++;
  });
}

/*---------------------------------------------------------
Fonction qui permet d'utiliser le contenue d'un fichier CSV.
Par défaut il utilise le fichier saveS.csv
---------------------------------------------------------*/
var position = [];
function chooseSave(filename){
  Papa.parse(filename, {
    header: false,
    download: true,
    delimiter: ";",
    dynamicTyping: true,
    complete: function(results) {
      if(filename == "save.csv"){
      displaySave(results); 
      }else{
        console.log(results);

        var count = 0;
        results.data[0].forEach(element => {
          var backgroundcolor = "rgb(" + element + ")";
          //console.log();
          document.getElementById("sous-menu"+((count+1).toString(10))).style.background = backgroundcolor;

          //document.getElementById(id).style.background = backgroundcolor;
          count++;
        });
      }
    }
  });
}

/*---------------------------------------------------------
Fonction qui selection dans le cercle de couleur la couleur
choisie par l'utilisateur dans les sauvegardes.
---------------------------------------------------------*/
function setSaveColor(id){
  Papa.parse('save.csv', {
    header: false,
    download: true,
    delimiter: ";",
    dynamicTyping: true,
    complete: function(results) {
      if(results.data[0][0] == null){
        var number = 1;
      }else{var number =0;}
      var tableau = results.data[number];
      colorPicker.color.rgbString = "rgb("+ tableau[id-1] +")";
    }
  });
}


function saving(){
  connection.send("sR"+red);
  connection.send("sG"+green);
  connection.send("sB"+blue);

}

setInterval(function() {
  if(on){

    connection.send("R"+red);
    connection.send("G"+green);
    connection.send("B"+blue);

  }
}, 500);

//Fonction qui permet de gérer les interactions de l'utilisateur sur la page.
document.addEventListener('DOMContentLoaded', function () {
  var SmartLight = document.querySelector('input[name=SmartLight]');
  var checkbox = document.querySelector('input[type="checkbox"]');
  var Reglage = document.querySelector('input[name=menu-open]');


  //Fonction qui detecte quand on veut régler la plage de couleur de l'éclairage intelligent
  Reglage.addEventListener('change', function () {
    if(Reglage.checked){
      console.log("yey");
      chooseSave("saveS.csv");
      rangeDefine = true;
    }else{
      rangeDefine = false;
      pickColor = 0;

      var couleurs = ["sTR","sTG","sTB"];

      for(var i=0;i<3;i++){
        console.log(couleurs[i] + ((((((document.getElementById("sous-menu2").style.background).split("("))[1]).split(")"))[0]).split(","))[i]);

        connection.send(couleurs[i] + ((((((document.getElementById("sous-menu1").style.background).split("("))[1]).split(")"))[0]).split(","))[i]);

      }
      for(var i=0;i<3;i++){
      connection.send(couleurs[i] + ((((((document.getElementById("sous-menu2").style.background).split("("))[1]).split(")"))[0]).split(","))[i]);
      }
    }
  });

  //Fonction qui detecte lorsque l'on affiche les couleurs sauvegardées.
  checkbox.addEventListener('change', function () {
    if (checkbox.checked) {
      on = true;
    } else {
      connection.send("R0");
      connection.send("G0");
      connection.send("B0");
      on = false;
    }
  });

  //Fonction qui detecte qu'on l'on active ou non l'éclairage intelligent.
  SmartLight.addEventListener('change', function () {
    if (SmartLight.checked) {
      if(on){
        document.getElementById("onOFf").checked = false;
        on = false;
      }
      connection.send("#1");
    } else {
      connection.send("#0");
      //on = true;
    }
  });
});

var colorPicker = new iro.ColorPicker(".colorPicker", {
  // color picker options
  // Option guide: https://iro.js.org/guide.html#color-picker-options
  width: 350,
  //color: "rgb(255, 0, 0)",
  borderWidth: 1,
  borderColor: "#fff",
});

 colorPicker.on(["color:init", "color:change"], function(color){

    red = ((color.rgbString).split(',')[0]).substring(4, 7);
    green = (color.rgbString).split(',')[1];
    var longueur =  ((color.rgbString).split(',')[2]).length;
    blue = (((color.rgbString).split(',')[2]).substring(0,longueur-1));

    if(rangeDefine && pickColor){
      var backgroundcolor = "rgb("+red+","+green+","+blue+")";
      document.getElementById("sous-menu"+pickColor).style.background = backgroundcolor;
    }
});
