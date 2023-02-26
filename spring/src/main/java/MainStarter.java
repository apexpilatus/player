import org.springframework.boot.SpringApplication;
import org.springframework.boot.SpringBootConfiguration;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.context.annotation.Import;

@SpringBootConfiguration
@EnableAutoConfiguration
@Import(MainPage.class)
public class MainStarter {
    public static void main(String[] args) {
        SpringApplication.run(MainStarter.class, args);
    }
}
