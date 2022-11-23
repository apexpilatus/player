function setvolume(direction){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("volume").innerHTML = "&#127911 " + this.responseText;
	}
	xhttp.open("POST", window.location.href.replace("player", "control") + "?volume=" + direction);
	xhttp.send();
}

function gettracks(album){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("tracks").src = this.responseText;
		document.getElementById("tracks").hidden = false;
		document.getElementById("picturebytes").hidden = false;
	}
	xhttp.open("GET", window.location.href.replace("player", "control") + "?album=" + album.replace(/&/g, " "));
	xhttp.send();
}

function play(album){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("picture").src = this.responseText;
		document.getElementById("tracks").hidden = true;
		document.getElementById("picturebytes").hidden = true;
	}
	xhttp.open("POST", window.location.href + "?album=" + album.replace(/&/g, " "));
	xhttp.send();
}

function makebig(){
    element = document.getElementById("picture");
    element.style="width:540px;height:540px;position:fixed;top:10px;left:5px;border-style:solid;border-color:black;";
    element.setAttribute("onclick", "makenormal()");
}

function makenormal(){
    element = document.getElementById("picture");
    element.style="width:250px;height:250px;position:fixed;top:300px;left:5px;border-style:solid;border-color:white;";
    element.setAttribute("onclick", "makebig()");
}
