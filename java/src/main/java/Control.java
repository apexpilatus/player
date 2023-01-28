import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.Arrays;
import java.util.Date;
import java.util.Objects;

public class Control extends HttpServlet implements Common {
    private synchronized void action1SetVol(int vol) {
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(currentPlayer[0], playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            byte op = 1;
            sockWriter.write(op);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(vol);
            sockWriter.flush();
            sockReader.readLine();
        } catch (IOException ignored) {
            currentPlayer[0] = "none";
        }
    }

    private synchronized int action2GetVol() {
        int ret = -1;
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(currentPlayer[0], playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            byte op = 2;
            writer.write(op);
            writer.flush();
            ret = reader.read();
        } catch (IOException ignored) {
            currentPlayer[0] = "none";
        }
        return ret;
    }

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) {
        String volChangeDirection = req.getParameter("volume");
        int vol = currentPlayer[0].equals("none") ? -1 : action2GetVol();
        if (!currentPlayer[0].equals("none") && volChangeDirection != null) {
            switch (volChangeDirection) {
                case "up" -> vol++;
                case "down" -> vol--;
                default -> {
                }
            }
            action1SetVol(vol);
            vol = action2GetVol();
        }
        resp.setContentType("text/plain");
        try {
            resp.getWriter().println(currentPlayer[0] + " " + vol);
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
                StringBuilder title = new StringBuilder("<head><meta charset=UTF-8></head>\n");
                title.append("<body style=background-color:lightgray;>\n");
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
                                        vorbisTitle[0] = line.substring(7);
                                    }
                                    if (file.equals("01.flac") && line.contains("ALBUM")) {
                                        vorbisAlbum[0] = line.substring(7);
                                    }
                                    if (file.equals("01.flac") && line.contains("ARTIST")) {
                                        vorbisArtist[0] = line.substring(8);
                                    }
                                });
                            }
                        }
                        if (file.equals("01.flac")) {
                            title.append("<p style=color:black;font-size:120%;><b>").append(vorbisArtist[0]).append("</b><br><strong style=color:slategray;>").append(vorbisAlbum[0]).append("</strong></p>\n");
                            title.append("</body>");
                        }
                        htmlFileWriter.write("<i onclick=play(\"" + albumToList.replace(" ", "&") + "\",\"" + file + "\")><small style=color:white;>" + vorbisTrack[0] + "</small>" + vorbisTitle[0] + "</i><br>\n");
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                htmlFileWriter.write("</p>\n");
                htmlFileWriter.write("<iframe height=115 width=450 style=position:fixed;top:0px;left:0px;border:none src=\"data:text/html," + title + "\"></iframe>\n");
            }
            htmlFileWriter.write("</body>\n");
            resp.getWriter().println(htmlName);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
