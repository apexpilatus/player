function setvolume(direction){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("volume").innerHTML = "&#127911 " + this.responseText;
	}
	xhttp.open("POST", window.location.href.replace("player", "control") + "?volume=" + direction);
	xhttp.send();
}

function switchdevice(){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("volume").innerHTML = "&#127911 " + this.responseText;
	}
    xhttp.open("POST", window.location.href.replace("player", "periodic"));
    xhttp.send();
	setInterval(function() {
		xhttp.open("POST", window.location.href.replace("player", "periodic"));
	    xhttp.send();
	}, 12000);
}

function gettracks(album){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("tracks").src = this.responseText;
		document.getElementById("tracks").hidden = false;
		document.getElementById("picturebytes").hidden = false;
		document.getElementById("hidebutton").hidden = false;
		makenormal();
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
		document.getElementById("hidebutton").hidden = true;
	}
	xhttp.open("POST", window.location.href + "?album=" + album.replace(/&/g, " "));
	xhttp.send();
}

function hideTracks(){
	document.getElementById("tracks").hidden = true;
	document.getElementById("picturebytes").hidden = true;
	document.getElementById("hidebutton").hidden = true;
}

function makebig(){
    element = document.getElementById("picture");
    element.style="width:540px;height:540px;position:fixed;top:10px;left:5px;border-style:solid;border-color:black;";
    element.setAttribute("onclick", "makenormal()");
    document.getElementById("picturebytes").hidden = true;
    document.getElementById("tracks").hidden = true;
    document.getElementById("hidebutton").hidden = true;
}

function makenormal(){
    element = document.getElementById("picture");
    element.style="width:250px;height:250px;position:fixed;top:300px;left:5px;border-style:solid;border-color:white;";
    element.setAttribute("onclick", "makebig()");
}
