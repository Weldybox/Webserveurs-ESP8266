

var on = true;

var red;
var green;
var blue;


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

var position = [];
function chooseSave(){
  Papa.parse('save.csv', {
    header: false,
    download: true,
    delimiter: ";",
    dynamicTyping: true,
    complete: function(results) {
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
  });
}

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
    /*var sauv = document.getElementById("sauv");
    sauv.style.backgroundColor = "rgb("+ red + "," + green + "," + blue + ")";*/

    connection.send("R"+red);
    connection.send("G"+green);
    connection.send("B"+blue);

  }
}, 500);

document.addEventListener('DOMContentLoaded', function () {
  var SmartLight = document.querySelector('input[name=SmartLight]');
  var checkbox = document.querySelector('input[type="checkbox"]');

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
        // if((color.rgbString).split(',')[2]) ==
    var longueur =  ((color.rgbString).split(',')[2]).length;
    blue = (((color.rgbString).split(',')[2]).substring(0,longueur-1));

});