import javax.sound.sampled.*;
import java.io.File;
import java.io.IOException;
import java.util.Scanner;

public class recorder{

    private static TargetDataLine mic;
    @SuppressWarnings("unused")
    private static boolean isRecording = false;

    public static void main(String[] args){
        @SuppressWarnings("resource")
        Scanner scan = new Scanner(System.in);

        AudioFormat format = new AudioFormat(AudioFormat.Encoding.PCM_SIGNED, 
        44100, 16, 1, 2, 44100, false);
        DataLine.Info info = new DataLine.Info(TargetDataLine.class, format);

        if(!AudioSystem.isLineSupported(info)){
            System.out.println("Audio format not supported");
        }

        try{
            mic = (TargetDataLine) AudioSystem.getLine(info);

            System.out.println("Enter name (without extension): ");
            String file = scan.nextLine().trim() + ".wav";

            File outputFile = new File(file);

            System.out.println("press space to start recording....");
            scan.nextLine();
            startRecording(format, outputFile);

            System.out.println("Press space bar to stop recording.... ");

            while(true){
                String input = scan.nextLine();
                if(input.trim().equalsIgnoreCase("")){
                    stopRecording();
                    System.out.println("Recording stopped. File saved");
                    break;
                }
            }
        }catch(LineUnavailableException e){
            System.err.println("Mic Unavailable: " + e.getMessage());
        }
    }
    private static void startRecording(AudioFormat format, File outputFile) {
        try{
            mic.open(format);
            mic.start();
            isRecording = true;

            Thread recThread = new Thread(() -> {
                try(AudioInputStream audio = new AudioInputStream(mic)){
                    AudioSystem.write(audio, AudioFileFormat.Type.WAVE, outputFile);
                } catch(IOException e){
                    System.err.println("Error while saving recording: " + e.getMessage());
                }
            });

            recThread.start();
        } catch(LineUnavailableException e){
            System.err.println("Error starting recording: " + e.getMessage());
        }
    }
    private static void stopRecording() {
        isRecording = false;
        mic.stop();
        mic.close();
    }
}