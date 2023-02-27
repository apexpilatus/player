package controllers;

import beans.Storage;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.List;
import java.util.Map;

@RestController
public class MainPage {
    @GetMapping("/")
    void mainPage(@RequestHeader("Host") String host, HttpServletResponse resp, Storage store) throws IOException {
        Map<String, List<String>> albums = store.getAlbums();
        resp.setContentType("text/html");
        resp.setCharacterEncoding("UTF-8");
        PrintWriter respWriter = resp.getWriter();
        respWriter.println("<!DOCTYPE html>");
        respWriter.println("<html>");
        respWriter.println("<head><meta charset=UTF-8><title>player</title>");
        respWriter.println("<link rel=apple-touch-icon href=apple-180x180.png sizes=180x180 type=image/png>");
        respWriter.println("<script>");
        respWriter.println("function play(album){");
        respWriter.println("\tconst xhttp = new XMLHttpRequest();");
        respWriter.println("\txhttp.onload = function() {");
        respWriter.println("\tdocument.getElementById(\"tracks\").hidden = true;");
        respWriter.println("\tdocument.getElementById(\"trackspicture\").hidden = true;");
        respWriter.println("\tdocument.getElementById(\"hidetracks\").hidden = true;");
        respWriter.println("\t}");
        respWriter.println("\txhttp.open(\"GET\", window.location.href + \"play\" + \"?album=\" + album.replace(/&/g, \" \"));");
        respWriter.println("\txhttp.send();");
        respWriter.println("}");
        respWriter.println("function setVolume(direction){");
        respWriter.println("\tconst xhttp = new XMLHttpRequest();");
        respWriter.println("\txhttp.onload = function() {");
        respWriter.println("\t\tdocument.getElementById(\"volume\").innerHTML = \"&#127911 \" + this.responseText;");
        respWriter.println("\t}");
        respWriter.println("\txhttp.open(\"GET\", window.location.href + \"volume\" + \"?direction=\" + direction);");
        respWriter.println("\txhttp.send();");
        respWriter.println("}");
        respWriter.println("function getTracks(album){");
        respWriter.println("\tdocument.getElementById(\"tracks\").src = \"http://" + host + "/album?album=\" + album.replace(/&/g, \" \")");
        respWriter.println("\tdocument.getElementById(\"tracks\").hidden = false;");
        respWriter.println("\tdocument.getElementById(\"trackspicture\").hidden = false;");
        respWriter.println("\tdocument.getElementById(\"hidetracks\").hidden = false;");
        respWriter.println("}");
        respWriter.println("function hideTracks(){");
        respWriter.println("\tdocument.getElementById(\"tracks\").hidden = true;");
        respWriter.println("\tdocument.getElementById(\"trackspicture\").hidden = true;");
        respWriter.println("\tdocument.getElementById(\"hidetracks\").hidden = true;");
        respWriter.println("}");
        respWriter.println("</script>");
        respWriter.println("</head>");
        respWriter.println("<body style=background-color:slategray;>");
        respWriter.println("<p id=volume style=position:fixed;top:420px;left:20px;font-size:20px;></p>");
        respWriter.println("<button type=button onclick=setVolume(\"up\") style=border-radius:20px;color:black;background-color:white;font-size:20px;position:fixed;top:50px;left:10px;>up</button>");
        respWriter.println("<button type=button onclick=setVolume(\"down\") style=border-radius:20px;color:black;background-color:white;font-size:20px;position:fixed;top:460px;left:10px;>dw</button>");
        respWriter.println("<script>setVolume(\"init\")</script>");
        respWriter.println("<iframe hidden id=tracks width=450 height=430 style=\"position:fixed;top:10px;right:10px;border-style:solid;\"></iframe>");
        respWriter.println("<img hidden id=trackspicture style=width:250px;height:250px;position:fixed;top:10px;right:470px;border-style:solid;>");
        respWriter.println("<button hidden type=button id=hidetracks onclick=hideTracks() style=border-radius:20%;color:red;background-color:black;font-size:25px;position:fixed;top:460px;right:10px;>X</button>");
        respWriter.println("<ul style=padding-left:150px;font-size:150%;line-height:180%;list-style-type:circle;>");
        albums.forEach((album, albumPathList) -> albumPathList.forEach((albumPath) -> respWriter.println("<li><b style=color:black; onclick=getTracks(\"" + (albumPath + "/" + album).replace(" ", "&") + "\")>" + album.replace("fuckingslash", "/").replace("fuckingblackstar", "&#9733").replace("fuckingplus", "&#43").replace(" anD ", " & ").replace("___", " <small style=color:white;>") + "</li></b></small>")));
        respWriter.println("</ul>");
        respWriter.println("</body>");
        respWriter.println("</html>");
    }
}