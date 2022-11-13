function play(album){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("picture").src = this.responseText;
	}
	xhttp.open("POST", window.location.href + "?album=" + album.replace(/&/g, " "));
	xhttp.send();
}

function volume(direction){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("volume").innerHTML = this.responseText;
	}
	xhttp.open("POST", window.location.href.replace("player", "control") + "?volume=" + direction);
	xhttp.send();
}

function tracks(album){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("tracks").innerHTML = this.responseText;
	}
	xhttp.open("POST", window.location.href.replace("player", "control") + "?album=" + album.replace(/&/g, " "));
	xhttp.send();
}
