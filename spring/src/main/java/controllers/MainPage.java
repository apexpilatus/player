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
                respWriter.println("<script src=\"main.js\"></script>");
                respWriter.println("</head>");
                respWriter.println("<body style=background-color:slategray;>");
                respWriter.println(
                                "<button id=showvolume onclick=getVolume() style=border-radius:20%;color:red;background-color:black;font-size:25px;position:fixed;top:460px;right:120px;><b>&#9738</b></button>");
                respWriter.println(
                                "<input hidden id=volume type=range onchange=setVolume() style=position:fixed;top:400px;width:500px;>");
                respWriter.println(
                                "<iframe hidden id=tracks width=450 height=430 style=\"position:fixed;top:10px;right:10px;border-style:solid;\"></iframe>");
                respWriter.println(
                                "<img hidden id=trackspicture style=width:320px;height:320px;position:fixed;top:10px;left:10px;border-style:solid;>");
                respWriter.println(
                                "<button hidden id=hidetracks onclick=hideTracks() style=border-radius:20%;color:red;background-color:black;font-size:25px;position:fixed;top:460px;right:10px;>X</button>");
                respWriter.println(
                                "<ul style=padding-left:50px;font-size:150%;line-height:180%;list-style-type:circle;>");
                albums.forEach((album, albumPathList) -> albumPathList.forEach((albumPath) -> respWriter.println(
                                "<li><b style=color:black; onclick=getTracks(\""
                                                + (albumPath + "/" + album).replace(" ", "&") + "\")>"
                                                + album.replace("fuckingslash", "/")
                                                                .replace("fuckingblackstar", "&#9733")
                                                                .replace("fuckingplus", "&#43").replace(" anD ", " & ")
                                                                .replace("___", " <small style=color:white;>")
                                                + "</li></b></small>")));
                respWriter.println("</ul>");
                respWriter.println("</body>");
                respWriter.println("</html>");
        }
}
