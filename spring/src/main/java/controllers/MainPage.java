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
                respWriter.println("<html lang=en>");
                respWriter.println("<head><meta charset=UTF-8><title>player</title>");
                respWriter.println("<link rel=apple-touch-icon href=apple-180x180.png sizes=180x180 type=image/png>");
                respWriter.println("<link rel=stylesheet href=mainstyle.css type=text/css>");
                respWriter.println("<script src=main.js></script>");
                respWriter.println("</head>");
                respWriter.println("<body>");
                respWriter.println("<button id=showvolume onclick=getVolume()>&#9738</button>");
                respWriter.println("<input hidden id=volume type=range onchange=setVolume() min=0 max=5 title=volume>");
                respWriter.println("<iframe hidden id=tracks title=meta></iframe>");
                respWriter.println("<img hidden id=trackspicture title=picture>");
                respWriter.println("<button hidden id=hidetracks onclick=hideTracks()>&#9737</button>");
                respWriter.println("<ul id=albums class=scroll>");
                albums.forEach((album, albumPathList) -> albumPathList.forEach(
                                (albumPath) -> respWriter.println("<li><b style=color:black; onclick=getTracks(\""
                                                + (albumPath + "/" + album).replace(" ", "&") + "\")>"
                                                + album.replace("fuckingslash", "/")
                                                                .replace("fuckingblackstar", "&#9733")
                                                                .replace("fuckingplus", "&#43").replace(" anD ", " & ")
                                                                .replace("___", " <small style=color:white;>")
                                                + (album.contains("___") ? "</small>" : "") + "</b></li>")));
                respWriter.println("</ul>");
                respWriter.println("</body>");
                respWriter.println("</html>");
        }
}
