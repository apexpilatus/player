function play(album, track){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		parent.document.getElementById("picture").src = this.responseText;
	}
	xhttp.open("POST", parent.window.location.href + "?album=" + album.replace(/&/g, " ") + "&track=" + track);
	xhttp.send();
}

function hideTracks(){
	parent.document.getElementById("tracks").hidden = true;
	parent.document.getElementById("picturebytes").hidden = true;
}

function getpicturebytes(album){
    const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		element = parent.document.getElementById("picturebytes");
		element.src = "data:image/jpeg;base64," + this.responseText;
		element.setAttribute("onclick", "play(\"" + album + "\")");
	}
	xhttp.open("GET", parent.window.location.href + "?album=" + album.replace(/&/g, " "));
	xhttp.send();
}