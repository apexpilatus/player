function play(album){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		document.getElementById("picture").src = this.responseText;
	}
	xhttp.open("POST", window.location.href + "?album=" + album.replace(/&/g, " "));
	xhttp.send();
}
