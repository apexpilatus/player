function getalbums() {
    document.getElementById("albums").src = window.location.href + "albums";
}

function getvolume(){
fetch("getvolume")
.then(resp => {
if (resp.status == 200) {
text=resp.text()
document.getElementById("volume").innerHTML = "&#9738";
            document.getElementById("control").min = text.split(";")[0];
            document.getElementById("control").value = text.split(";")[1];
            document.getElementById("control").max = text.split(";")[2];
            document.getElementById("control").hidden = false;
} else {
document.getElementById("volume").innerHTML = "&#9739";
}
})}
