function hidetracks(){
    document.getElementById("hidetracks").hidden = true;
    document.body.style.overflow = "auto";
    document.getElementById("albums").setAttribute("class", "clear");
    document.getElementById("tracks").hidden = true;
}

function gettracks(album){
    document.getElementById("tracks").src = window.location.href + "gettracks?" + album;
    document.getElementById("hidetracks").hidden = false;
    document.body.style.overflow = "hidden";
    document.getElementById("albums").setAttribute("class", "blur");
}
