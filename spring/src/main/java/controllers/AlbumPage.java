package controllers;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class AlbumPage {
    @GetMapping("/album")
    void albumPage() {
    }
}