function setvolume(direction){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("volume").innerHTML = this.responseText;
	}
	xhttp.open("POST", window.location.href.replace("player", "control") + "?volume=" + direction);
	xhttp.send();
}

function gettracks(album){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("tracks").src = this.responseText;
		document.getElementById("tracks").hidden = false;
	}
	xhttp.open("GET", window.location.href.replace("player", "control") + "?album=" + album.replace(/&/g, " "));
	xhttp.send();
}
