package controllers;

import beans.Storage;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.Map;
import java.util.Objects;

@RestController
public class AlbumPage {
    @GetMapping("/album")
    void albumPage(@RequestParam("album") String album, HttpServletResponse resp, Storage store) throws IOException {
        resp.setContentType("text/html");
        resp.setCharacterEncoding("UTF-8");
        PrintWriter respWriter = resp.getWriter();
        respWriter.println("<!DOCTYPE html>");
        respWriter.println("<html>");
        respWriter.println("<head>");
        respWriter.println("<meta charset=UTF-8>");
        respWriter.println("<script>");
        respWriter.println("function gettrackspicture(album){");
        respWriter.println("\tconst xhttp = new XMLHttpRequest();");
        respWriter.println("\txhttp.onload = function() {");
        respWriter.println("\t\telement = parent.document.getElementById(\"trackspicture\");");
        respWriter.println("\t\telement.src = \"data:image/jpeg;base64,\" + this.responseText;");
        respWriter.println("\t\telement.setAttribute(\"onclick\", \"play(\\\"\" + album + \"\\\")\");");
        respWriter.println("\t}");
        respWriter.println("\txhttp.open(\"GET\", parent.window.location.href + \"player\" + \"?album=\" + album.replace(/&/g, \" \"));");
        respWriter.println("\txhttp.send();");
        respWriter.println("}");
        respWriter.println("</script>");
        respWriter.println("</head>");
        respWriter.println("<body style=background-color:gray;>");
        respWriter.println("<p style=padding-top:120px;font-size:120%;line-height:180%>");
        File albumDirPath = new File(album);
        String[] files = albumDirPath.list();
        Arrays.sort(Objects.requireNonNull(files));
        StringBuilder title = new StringBuilder("<!DOCTYPE html><html><head><meta charset=UTF-8></head>\n");
        title.append("<body style=background-color:lightgray;>\n");
        for (String file : files) {
            Map<String, String> metasMap = store.getMetas(album + "/" + file);
            if (file.equals("01.flac")) {
                title.append("<p style=color:black;font-size:120%;><b>").append(metasMap.get("ARTIST") == null ? "" : metasMap.get("ARTIST")).append("</b><br><strong style=color:slategray;>").append(metasMap.get("ALBUM")).append("</strong></p>\n");
                title.append("</body></html>");
            }
            respWriter.println("<i onclick=play(\"" + album.replace(" ", "&") + "\",\"" + file + "\")><small style=color:white;>" + metasMap.get("TRACKNUMBER") + "</small>" + metasMap.get("TITLE") + "</i><br>");
        }
        respWriter.println("</p>");
        respWriter.println("<iframe height=115 width=450 style=position:fixed;top:0px;left:0px;border:none src=\"data:text/html," + title + "\"></iframe>\n");
        respWriter.println("</body>");
        respWriter.println("</html>");
    }
}