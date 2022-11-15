function play(album, track){
	const xhttp = new XMLHttpRequest();
	xhttp.onload = function() {
		parent.document.getElementById("picture").src = this.responseText;
		if(track == "all"){
			parent.document.getElementById("tracks").hidden = true;
		}
	}
	xhttp.open("POST", parent.window.location.href + "?album=" + album.replace(/&/g, " ") + "&track=" + track);
	xhttp.send();
}

function hideTracks(){
	parent.document.getElementById("tracks").hidden = true;
}
