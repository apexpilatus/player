import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;

import java.io.*;
import java.util.Arrays;
import java.util.Date;
import java.util.Objects;

public class Control extends HttpServlet {
    String volumeFilePath = "/home/exe/player/tmp/volume";
    String exeDirPath = "/home/exe";

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) {
        String volChangeDirection = req.getParameter("volume");
        File volFile = new File(volumeFilePath);
        byte[] vol = {5};
        if (volChangeDirection != null) {
            if (volFile.exists()) {
                try (FileInputStream volumeFileReader = new FileInputStream(volumeFilePath)) {
                    volumeFileReader.read(vol);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            switch (volChangeDirection) {
                case "up" -> vol[0]++;
                case "down" -> vol[0]--;
                default -> {
                }
            }
            try (FileOutputStream albumFileWriter = new FileOutputStream(volumeFilePath)) {
                albumFileWriter.write(vol);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        resp.setContentType("text/plain");
        try {
            resp.getWriter().println(vol[0]);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) {
        String albumToList = req.getParameter("album");
        String htmlName = "frame.html";
        String htmlDirPath = exeDirPath + "/player/tmp";
        if (albumToList != null) {
            File dirFile = new File(exeDirPath);
            for (String file : Objects.requireNonNull(dirFile.list())) {
                if (file.contains("apache-tom")) {
                    htmlDirPath = exeDirPath + "/" + file + "/" + "webapps/ROOT";
                }
            }
            dirFile = new File(htmlDirPath);
            for (File file : Objects.requireNonNull(dirFile.listFiles())) {
                if (file.getName().contains("html") && !file.getName().contains("index")) {
                    htmlName = file.getName();
                    if (file.delete()) {
                        htmlName = new Date().getTime() + ".html";
                    }
                }
            }
        }
        resp.setContentType("text/plain");
        try (BufferedWriter htmlFileWriter = new BufferedWriter(new FileWriter(htmlDirPath + "/" + htmlName))) {
            htmlFileWriter.write("<head><meta charset=UTF-8></head>\n");
            htmlFileWriter.write("<body style=background-color:gray;>\n");
            htmlFileWriter.write("<script src=tracks.js></script>\n");
            if (albumToList != null) {
                htmlFileWriter.write("<script>getpicturebytes(\"" + albumToList.replace(" ", "&") + "\")</script>\n");
                File albumDirPath = new File(albumToList);
                String[] files = albumDirPath.list();
                Arrays.sort(Objects.requireNonNull(files));
                htmlFileWriter.write("<p style=padding-top:120px;font-size:120%;line-height:180%>\n");
                String title = "<head><meta charset=UTF-8></head>\n";
                title += "<body style=background-color:lightgray;>\n";
                for (String file : files) {
                    try (FileInputStream flacIs = new FileInputStream(albumToList + "/" + file)) {
                        FLACDecoder flacDec = new FLACDecoder(flacIs);
                        Metadata[] metas = flacDec.readMetadata();
                        final String[] vorbisTrack = {""};
                        final String[] vorbisTitle = {""};
                        final String[] vorbisAlbum = {""};
                        final String[] vorbisArtist = {""};
                        for (Metadata meta : metas) {
                            if (meta.toString().contains("VorbisComment")) {
                                meta.toString().lines().forEach((line) -> {
                                    if (line.contains("TRACKNUMBER")) {
                                        vorbisTrack[0] = line.split("=")[1] + ". ";
                                    }
                                    if (line.contains("TITLE")) {
                                    	for (int i = 1; i < line.split("=").length; i++){
                                    	    if (i > 1){
                                    	        vorbisTitle[0] += "=";
                                    	    }
                                            vorbisTitle[0] += line.split("=")[i];
                                        }
                                    }
                                    if (line.contains("ALBUM")) {
                                        for (int i = 1; i < line.split("=").length; i++){
                                            if (i > 1){
                                    	        vorbisAlbum[0] += "=";
                                    	    }
                                            vorbisAlbum[0] += line.split("=")[i];
                                        }
                                    }
                                    if (line.contains("ARTIST")) {
                                        for (int i = 1; i < line.split("=").length; i++){
                                            if (i > 1){
                                    	        vorbisArtist[0] += "=";
                                    	    }
                                            vorbisArtist[0] += line.split("=")[i];
                                        }
                                    }
                                });
                            }
                        }
                        if (file.equals("01.flac")) {
                            title += "<p style=color:black;font-size:120%;><b>" + vorbisArtist[0] + "</b><br><strong style=color:slategray;>" + vorbisAlbum[0] + "</strong></p>\n";
                            title += "</body>";
                        }
                        htmlFileWriter.write("<i onclick=play(\"" + albumToList.replace(" ", "&") + "\",\"" + file + "\")><small style=color:white;>" + vorbisTrack[0] + "</small>" + vorbisTitle[0] + "</i><br>\n");
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                htmlFileWriter.write("</p>\n");
                htmlFileWriter.write("<iframe height=115 width=500 style=position:fixed;top:0px;left:0px;border:none src=\"data:text/html," + title + "\"></iframe>\n");
            }
            htmlFileWriter.write("</body>\n");
            resp.getWriter().println(htmlName);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
