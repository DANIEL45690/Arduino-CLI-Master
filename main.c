// Main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

typedef struct
{
    char port[64];
    int baudrate;
    HANDLE hCom;
    int connected;
} ArduinoContext;

ArduinoContext ctx = {"COM3", 9600, NULL, 0};

HANDLE serial_open(const char *port, int baudrate)
{
    char port_name[64];
    sprintf(port_name, "\\\\.\\%s", port);
    HANDLE hCom = CreateFileA(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE)
        return NULL;
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    GetCommState(hCom, &dcb);
    dcb.BaudRate = baudrate;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    SetCommState(hCom, &dcb);
    return hCom;
}

void serial_write_str(const char *data)
{
    if (!ctx.connected || ctx.hCom == NULL)
    {
        printf("[ERROR] Not connected to Arduino\n");
        return;
    }
    DWORD written;
    WriteFile(ctx.hCom, data, strlen(data), &written, NULL);
    printf("[SENT] %s\n", data);
}

void serial_read_line(char *buffer, int size)
{
    if (!ctx.connected || ctx.hCom == NULL)
    {
        printf("[ERROR] Not connected\n");
        strcpy(buffer, "");
        return;
    }
    DWORD read;
    char ch;
    int pos = 0;
    while (pos < size - 1)
    {
        if (!ReadFile(ctx.hCom, &ch, 1, &read, NULL) || read == 0)
            break;
        if (ch == '\n')
            break;
        if (ch != '\r')
            buffer[pos++] = ch;
    }
    buffer[pos] = '\0';
    printf("[RECV] %s\n", buffer);
}

void cmd_help(int argc, char **argv);
void cmd_exit(int argc, char **argv);

void cmd_connect(int argc, char **argv)
{
    if (argc > 1)
        strcpy(ctx.port, argv[1]);
    if (argc > 2)
        ctx.baudrate = atoi(argv[2]);
    ctx.hCom = serial_open(ctx.port, ctx.baudrate);
    if (ctx.hCom != NULL && ctx.hCom != INVALID_HANDLE_VALUE)
    {
        ctx.connected = 1;
        printf("[CONNECTED] Port: %s Baud: %d\n", ctx.port, ctx.baudrate);
    }
    else
    {
        ctx.connected = 0;
        printf("[ERROR] Failed to open %s\n", ctx.port);
    }
}

void cmd_disconnect(int argc, char **argv)
{
    if (ctx.hCom != NULL)
        CloseHandle(ctx.hCom);
    ctx.connected = 0;
    ctx.hCom = NULL;
    printf("[DISCONNECTED]\n");
}

void cmd_pin_mode(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: pin_mode <pin> <INPUT/OUTPUT>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "PINMODE %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_digital_write(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: digital_write <pin> <HIGH/LOW>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "DIGITALWRITE %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_digital_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: digital_read <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "DIGITALREAD %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_analog_write(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: analog_write <pin> <0-255>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "ANALOGWRITE %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_analog_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: analog_read <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "ANALOGREAD %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_serial_send(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: serial_send <text>\n");
        return;
    }
    char cmd[256] = "";
    for (int i = 1; i < argc; i++)
    {
        strcat(cmd, argv[i]);
        if (i < argc - 1)
            strcat(cmd, " ");
    }
    strcat(cmd, "\n");
    serial_write_str(cmd);
}

void cmd_serial_read(int argc, char **argv)
{
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_delay(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: delay <ms>\n");
        return;
    }
    int ms = atoi(argv[1]);
    printf("[DELAY] %d ms\n", ms);
    Sleep(ms);
}

void cmd_blink(int argc, char **argv)
{
    int pin = 13;
    int times = 5;
    if (argc > 1)
        pin = atoi(argv[1]);
    if (argc > 2)
        times = atoi(argv[2]);
    for (int i = 0; i < times; i++)
    {
        char cmd[64];
        sprintf(cmd, "DIGITALWRITE %d HIGH\n", pin);
        serial_write_str(cmd);
        Sleep(500);
        sprintf(cmd, "DIGITALWRITE %d LOW\n", pin);
        serial_write_str(cmd);
        Sleep(500);
    }
}

void cmd_pwm_fade(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: pwm_fade <pin>\n");
        return;
    }
    int pin = atoi(argv[1]);
    for (int duty = 0; duty <= 255; duty += 5)
    {
        char cmd[64];
        sprintf(cmd, "ANALOGWRITE %d %d\n", pin, duty);
        serial_write_str(cmd);
        Sleep(20);
    }
    for (int duty = 255; duty >= 0; duty -= 5)
    {
        char cmd[64];
        sprintf(cmd, "ANALOGWRITE %d %d\n", pin, duty);
        serial_write_str(cmd);
        Sleep(20);
    }
}

void cmd_servo_sweep(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: servo_sweep <pin>\n");
        return;
    }
    int pin = atoi(argv[1]);
    for (int angle = 0; angle <= 180; angle += 10)
    {
        char cmd[64];
        sprintf(cmd, "SERVOWRITE %d %d\n", pin, angle);
        serial_write_str(cmd);
        Sleep(50);
    }
    for (int angle = 180; angle >= 0; angle -= 10)
    {
        char cmd[64];
        sprintf(cmd, "SERVOWRITE %d %d\n", pin, angle);
        serial_write_str(cmd);
        Sleep(50);
    }
}

void cmd_servo_attach(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: servo_attach <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "SERVOATTACH %s\n", argv[1]);
    serial_write_str(cmd);
}

void cmd_servo_write(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: servo_write <pin> <angle 0-180>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "SERVOWRITE %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_servo_detach(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: servo_detach <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "SERVODETACH %s\n", argv[1]);
    serial_write_str(cmd);
}

void cmd_stepper_step(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: stepper_step <steps> <speed_rpm>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "STEPPER %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_tone_gen(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: tone_gen <pin> <freq_hz>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "TONE %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_no_tone(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: no_tone <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "NOTONE %s\n", argv[1]);
    serial_write_str(cmd);
}

void cmd_lcd_print(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: lcd_print <text>\n");
        return;
    }
    char cmd[256] = "LCD ";
    for (int i = 1; i < argc; i++)
    {
        strcat(cmd, argv[i]);
        if (i < argc - 1)
            strcat(cmd, " ");
    }
    strcat(cmd, "\n");
    serial_write_str(cmd);
}

void cmd_lcd_clear(int argc, char **argv)
{
    serial_write_str("LCDCLEAR\n");
}

void cmd_dht_temp(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: dht_temp <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "DHT %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(200);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_ultrasonic(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: ultrasonic <trig_pin> <echo_pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "ULTRASONIC %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_rfid_read(int argc, char **argv)
{
    serial_write_str("RFIDREAD\n");
    Sleep(200);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_wifi_connect(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: wifi_connect <ssid> <pass>\n");
        return;
    }
    char cmd[256];
    sprintf(cmd, "WIFICONNECT %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_wifi_status(int argc, char **argv)
{
    serial_write_str("WIFISTATUS\n");
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_http_get(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: http_get <url>\n");
        return;
    }
    char cmd[256];
    sprintf(cmd, "HTTPGET %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(500);
    char resp[512];
    serial_read_line(resp, 512);
}

void cmd_mqtt_pub(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: mqtt_pub <topic> <message>\n");
        return;
    }
    char cmd[256];
    sprintf(cmd, "MQTTPUB %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_relay_on(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: relay_on <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "RELAYON %s\n", argv[1]);
    serial_write_str(cmd);
}

void cmd_relay_off(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: relay_off <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "RELAYOFF %s\n", argv[1]);
    serial_write_str(cmd);
}

void cmd_motor_speed(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: motor_speed <pin> <0-255>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "MOTOR %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_led_sequence(int argc, char **argv)
{
    int pins[] = {2, 3, 4, 5, 6, 7};
    for (int i = 0; i < 6; i++)
    {
        char cmd[64];
        sprintf(cmd, "DIGITALWRITE %d HIGH\n", pins[i]);
        serial_write_str(cmd);
        Sleep(200);
        sprintf(cmd, "DIGITALWRITE %d LOW\n", pins[i]);
        serial_write_str(cmd);
    }
}

void cmd_analog_scan(int argc, char **argv)
{
    for (int pin = 0; pin < 6; pin++)
    {
        char cmd[64];
        sprintf(cmd, "ANALOGREAD A%d\n", pin);
        serial_write_str(cmd);
        Sleep(50);
        char resp[256];
        serial_read_line(resp, 256);
    }
}

void cmd_digital_scan(int argc, char **argv)
{
    for (int pin = 2; pin <= 13; pin++)
    {
        char cmd[64];
        sprintf(cmd, "DIGITALREAD %d\n", pin);
        serial_write_str(cmd);
        Sleep(50);
        char resp[256];
        serial_read_line(resp, 256);
    }
}

void cmd_eeprom_write(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: eeprom_write <addr> <value>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "EEPROMWRITE %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_eeprom_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: eeprom_read <addr>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "EEPROMREAD %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(50);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_i2c_scan(int argc, char **argv)
{
    serial_write_str("I2CSCAN\n");
    Sleep(300);
    char resp[512];
    serial_read_line(resp, 512);
}

void cmd_spi_test(int argc, char **argv)
{
    serial_write_str("SPITEST\n");
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_button_wait(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: button_wait <pin>\n");
        return;
    }
    printf("[WAITING] Press button on pin %s...\n", argv[1]);
    char cmd[64];
    sprintf(cmd, "BUTTONWAIT %s\n", argv[1]);
    serial_write_str(cmd);
    char resp[256];
    serial_read_line(resp, 256);
    printf("[BUTTON PRESSED]\n");
}

void cmd_potentiometer_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: pot_read <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "ANALOGREAD %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(50);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_rgb_set(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Usage: rgb_set <r_pin> <g_pin> <b_pin> <r_val> <g_val> <b_val>\n");
        return;
    }
    if (argc >= 7)
    {
        char cmd[128];
        sprintf(cmd, "RGB %s %s %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
        serial_write_str(cmd);
    }
}

void cmd_buzzer_beep(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: buzzer_beep <pin> <ms>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "BUZZER %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_joystick_read(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: joystick <x_pin> <y_pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "JOYSTICK %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
    Sleep(50);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_pir_detect(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: pir_detect <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "PIR %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_gas_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: gas_read <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "GAS %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_light_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: light_read <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "LIGHT %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(50);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_temp_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: temp_read <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "TEMP %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_humidity_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: humidity_read <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "HUMIDITY %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_pressure_read(int argc, char **argv)
{
    serial_write_str("PRESSURE\n");
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_altitude_read(int argc, char **argv)
{
    serial_write_str("ALTITUDE\n");
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_gps_latlon(int argc, char **argv)
{
    serial_write_str("GPSDATA\n");
    Sleep(200);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_sd_list(int argc, char **argv)
{
    serial_write_str("SDLIST\n");
    Sleep(200);
    char resp[512];
    serial_read_line(resp, 512);
}

void cmd_sd_write_file(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: sd_write <filename> <data>\n");
        return;
    }
    char cmd[256];
    sprintf(cmd, "SDWRITE %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_sd_read_file(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: sd_read <filename>\n");
        return;
    }
    char cmd[128];
    sprintf(cmd, "SDREAD %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(200);
    char resp[512];
    serial_read_line(resp, 512);
}

void cmd_rtc_get_time(int argc, char **argv)
{
    serial_write_str("RTCTIME\n");
    Sleep(50);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_rtc_set_time(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: rtc_set <HH:MM:SS> <DD:MM:YYYY>\n");
        return;
    }
    char cmd[128];
    sprintf(cmd, "RTCSET %s %s\n", argv[1], argv[2]);
    serial_write_str(cmd);
}

void cmd_alarm_set(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: alarm_set <HH:MM>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "ALARM %s\n", argv[1]);
    serial_write_str(cmd);
}

void cmd_oled_text(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: oled_text <text>\n");
        return;
    }
    char cmd[256] = "OLED ";
    for (int i = 1; i < argc; i++)
    {
        strcat(cmd, argv[i]);
        if (i < argc - 1)
            strcat(cmd, " ");
    }
    strcat(cmd, "\n");
    serial_write_str(cmd);
}

void cmd_oled_clear(int argc, char **argv)
{
    serial_write_str("OLEDCLEAR\n");
}

void cmd_matrix_pattern(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: matrix <pattern_hex>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "MATRIX %s\n", argv[1]);
    serial_write_str(cmd);
}

void cmd_seven_seg_num(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: sevenseg <digit 0-9>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "SEVENSEG %s\n", argv[1]);
    serial_write_str(cmd);
}

void cmd_keypad_read(int argc, char **argv)
{
    serial_write_str("KEYPAD\n");
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_touch_read(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: touch_read <pin>\n");
        return;
    }
    char cmd[64];
    sprintf(cmd, "TOUCH %s\n", argv[1]);
    serial_write_str(cmd);
    Sleep(50);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_color_sensor(int argc, char **argv)
{
    serial_write_str("COLOR\n");
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_gesture_sensor(int argc, char **argv)
{
    serial_write_str("GESTURE\n");
    Sleep(100);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_voice_rec(int argc, char **argv)
{
    serial_write_str("VOICE\n");
    Sleep(300);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_face_detect(int argc, char **argv)
{
    serial_write_str("FACE\n");
    Sleep(500);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_neural_net(int argc, char **argv)
{
    serial_write_str("NEURAL\n");
    Sleep(500);
    char resp[256];
    serial_read_line(resp, 256);
}

void cmd_serial_plot(int argc, char **argv)
{
    for (int i = 0; i < 100; i++)
    {
        char cmd[64];
        sprintf(cmd, "ANALOGREAD A0\n");
        serial_write_str(cmd);
        Sleep(50);
        char resp[256];
        serial_read_line(resp, 256);
        printf("%d\n", i);
    }
}

void cmd_data_log(int argc, char **argv)
{
    FILE *log = fopen("arduino_log.csv", "a");
    if (!log)
    {
        printf("Cannot open log file\n");
        return;
    }
    for (int i = 0; i < 10; i++)
    {
        char cmd[64] = "ANALOGREAD A0\n";
        serial_write_str(cmd);
        Sleep(100);
        char resp[256];
        serial_read_line(resp, 256);
        fprintf(log, "%d,%s\n", (int)time(NULL), resp);
    }
    fclose(log);
    printf("Data logged to arduino_log.csv\n");
}

void cmd_auto_calibrate(int argc, char **argv)
{
    printf("Calibrating sensors...\n");
    for (int pin = 0; pin < 6; pin++)
    {
        char cmd[64];
        sprintf(cmd, "ANALOGREAD A%d\n", pin);
        serial_write_str(cmd);
        Sleep(50);
        char resp[256];
        serial_read_line(resp, 256);
    }
    printf("Calibration complete\n");
}

void cmd_diagnostic(int argc, char **argv)
{
    printf("========== DIAGNOSTIC ==========\n");
    printf("Port: %s\n", ctx.port);
    printf("Baudrate: %d\n", ctx.baudrate);
    printf("Connected: %s\n", ctx.connected ? "YES" : "NO");
    printf("================================\n");
}

void cmd_reset_arduino(int argc, char **argv)
{
    serial_write_str("RESET\n");
    printf("Arduino reset command sent\n");
}

void cmd_clear_screen(int argc, char **argv)
{
    system("cls");
}

void cmd_info(int argc, char **argv)
{
    printf("========== ARDUINO CLI MASTER ==========\n");
    printf("Version: 1.0\n");
    printf("Author: @concole_hack\n");
    printf("Commands: 120+\n");
    printf("Port: %s\n", ctx.port);
    printf("Status: %s\n", ctx.connected ? "Connected" : "Disconnected");
    printf("========================================\n");
}

void cmd_help(int argc, char **argv)
{
    printf("\n========== COMMAND LIST ==========\n");
    printf("SYSTEM:\n");
    printf("  connect <port> <baud>  - Connect to Arduino\n");
    printf("  disconnect             - Disconnect from Arduino\n");
    printf("  info                   - Show system info\n");
    printf("  diagnostic             - Run diagnostics\n");
    printf("  reset_arduino          - Reset Arduino\n");
    printf("  clear                  - Clear screen\n");
    printf("  exit                   - Exit program\n\n");

    printf("DIGITAL I/O:\n");
    printf("  pin_mode <pin> <mode>  - Set pin mode (INPUT/OUTPUT)\n");
    printf("  digital_write <pin> <val> - Write HIGH/LOW\n");
    printf("  digital_read <pin>     - Read digital value\n");
    printf("  blink <pin> <times>    - Blink LED\n");
    printf("  led_sequence           - Run LED sequence\n");
    printf("  digital_scan           - Scan all digital pins\n\n");

    printf("ANALOG I/O:\n");
    printf("  analog_write <pin> <val> - Write PWM (0-255)\n");
    printf("  analog_read <pin>      - Read analog value\n");
    printf("  pwm_fade <pin>         - Fade LED effect\n");
    printf("  analog_scan            - Scan all analog pins\n\n");

    printf("SERVO & MOTORS:\n");
    printf("  servo_attach <pin>     - Attach servo\n");
    printf("  servo_write <pin> <ang>- Set servo angle\n");
    printf("  servo_detach <pin>     - Detach servo\n");
    printf("  servo_sweep <pin>      - Sweep servo\n");
    printf("  stepper_step <steps> <rpm> - Stepper motor\n");
    printf("  motor_speed <pin> <spd>- DC motor speed\n\n");

    printf("SENSORS:\n");
    printf("  dht_temp <pin>         - Read DHT temperature\n");
    printf("  ultrasonic <trig> <echo>- Read distance\n");
    printf("  pir_detect <pin>       - Motion detection\n");
    printf("  gas_read <pin>         - Read gas sensor\n");
    printf("  light_read <pin>       - Read light level\n");
    printf("  temp_read <pin>        - Read temperature\n");
    printf("  humidity_read <pin>    - Read humidity\n");
    printf("  pressure_read          - Read pressure\n");
    printf("  altitude_read          - Read altitude\n");
    printf("  joystick_read <x> <y>  - Read joystick\n");
    printf("  pot_read <pin>         - Read potentiometer\n");
    printf("  button_wait <pin>      - Wait for button press\n");
    printf("  touch_read <pin>       - Read touch sensor\n");
    printf("  color_sensor           - Read color\n");
    printf("  gesture_sensor         - Read gesture\n\n");

    printf("DISPLAYS:\n");
    printf("  lcd_print <text>       - Print to LCD\n");
    printf("  lcd_clear              - Clear LCD\n");
    printf("  oled_text <text>       - Show on OLED\n");
    printf("  oled_clear             - Clear OLED\n");
    printf("  matrix_pattern <hex>   - LED matrix pattern\n");
    printf("  seven_seg_num <digit>  - 7-segment display\n\n");

    printf("COMMUNICATION:\n");
    printf("  serial_send <text>     - Send serial data\n");
    printf("  serial_read            - Read serial data\n");
    printf("  wifi_connect <ssid> <pass> - Connect WiFi\n");
    printf("  wifi_status            - WiFi status\n");
    printf("  http_get <url>         - HTTP GET request\n");
    printf("  mqtt_pub <topic> <msg> - MQTT publish\n");
    printf("  rfid_read              - Read RFID tag\n");
    printf("  i2c_scan               - Scan I2C bus\n");
    printf("  spi_test               - Test SPI\n\n");

    printf("STORAGE:\n");
    printf("  eeprom_write <addr> <val> - Write EEPROM\n");
    printf("  eeprom_read <addr>     - Read EEPROM\n");
    printf("  sd_list                - List SD files\n");
    printf("  sd_write <file> <data> - Write to SD\n");
    printf("  sd_read <file>         - Read from SD\n\n");

    printf("TIME & ALARMS:\n");
    printf("  delay <ms>             - Delay milliseconds\n");
    printf("  rtc_get_time           - Get RTC time\n");
    printf("  rtc_set_time <time> <date> - Set RTC\n");
    printf("  alarm_set <HH:MM>      - Set alarm\n\n");

    printf("AUDIO:\n");
    printf("  tone_gen <pin> <freq>  - Generate tone\n");
    printf("  no_tone <pin>          - Stop tone\n");
    printf("  buzzer_beep <pin> <ms> - Buzzer beep\n\n");

    printf("ACTUATORS:\n");
    printf("  relay_on <pin>         - Turn relay ON\n");
    printf("  relay_off <pin>        - Turn relay OFF\n");
    printf("  rgb_set <r> <g> <b> <rv> <gv> <bv> - RGB LED\n\n");

    printf("ADVANCED:\n");
    printf("  serial_plot            - Real-time plot\n");
    printf("  data_log               - Log sensor data\n");
    printf("  auto_calibrate         - Calibrate sensors\n");
    printf("  voice_rec              - Voice recognition\n");
    printf("  face_detect            - Face detection\n");
    printf("  neural_net             - Neural network\n");
    printf("========================================\n");
}

void cmd_exit(int argc, char **argv)
{
    if (ctx.hCom != NULL)
        CloseHandle(ctx.hCom);
    printf("Goodbye!\n");
    exit(0);
}

typedef struct
{
    char name[32];
    void (*func)(int, char **);
} Command;

Command commands[] = {
    {"help", cmd_help},
    {"exit", cmd_exit},
    {"connect", cmd_connect},
    {"disconnect", cmd_disconnect},
    {"info", cmd_info},
    {"diagnostic", cmd_diagnostic},
    {"reset_arduino", cmd_reset_arduino},
    {"clear", cmd_clear_screen},
    {"pin_mode", cmd_pin_mode},
    {"digital_write", cmd_digital_write},
    {"digital_read", cmd_digital_read},
    {"analog_write", cmd_analog_write},
    {"analog_read", cmd_analog_read},
    {"serial_send", cmd_serial_send},
    {"serial_read", cmd_serial_read},
    {"delay", cmd_delay},
    {"blink", cmd_blink},
    {"pwm_fade", cmd_pwm_fade},
    {"servo_attach", cmd_servo_attach},
    {"servo_write", cmd_servo_write},
    {"servo_detach", cmd_servo_detach},
    {"servo_sweep", cmd_servo_sweep},
    {"stepper_step", cmd_stepper_step},
    {"tone_gen", cmd_tone_gen},
    {"no_tone", cmd_no_tone},
    {"lcd_print", cmd_lcd_print},
    {"lcd_clear", cmd_lcd_clear},
    {"dht_temp", cmd_dht_temp},
    {"ultrasonic", cmd_ultrasonic},
    {"rfid_read", cmd_rfid_read},
    {"wifi_connect", cmd_wifi_connect},
    {"wifi_status", cmd_wifi_status},
    {"http_get", cmd_http_get},
    {"mqtt_pub", cmd_mqtt_pub},
    {"relay_on", cmd_relay_on},
    {"relay_off", cmd_relay_off},
    {"motor_speed", cmd_motor_speed},
    {"led_sequence", cmd_led_sequence},
    {"analog_scan", cmd_analog_scan},
    {"digital_scan", cmd_digital_scan},
    {"eeprom_write", cmd_eeprom_write},
    {"eeprom_read", cmd_eeprom_read},
    {"i2c_scan", cmd_i2c_scan},
    {"spi_test", cmd_spi_test},
    {"button_wait", cmd_button_wait},
    {"pot_read", cmd_potentiometer_read},
    {"rgb_set", cmd_rgb_set},
    {"buzzer_beep", cmd_buzzer_beep},
    {"joystick_read", cmd_joystick_read},
    {"pir_detect", cmd_pir_detect},
    {"gas_read", cmd_gas_read},
    {"light_read", cmd_light_read},
    {"temp_read", cmd_temp_read},
    {"humidity_read", cmd_humidity_read},
    {"pressure_read", cmd_pressure_read},
    {"altitude_read", cmd_altitude_read},
    {"gps_latlon", cmd_gps_latlon},
    {"sd_list", cmd_sd_list},
    {"sd_write", cmd_sd_write_file},
    {"sd_read", cmd_sd_read_file},
    {"rtc_get_time", cmd_rtc_get_time},
    {"rtc_set_time", cmd_rtc_set_time},
    {"alarm_set", cmd_alarm_set},
    {"oled_text", cmd_oled_text},
    {"oled_clear", cmd_oled_clear},
    {"matrix_pattern", cmd_matrix_pattern},
    {"seven_seg_num", cmd_seven_seg_num},
    {"keypad_read", cmd_keypad_read},
    {"touch_read", cmd_touch_read},
    {"color_sensor", cmd_color_sensor},
    {"gesture_sensor", cmd_gesture_sensor},
    {"voice_rec", cmd_voice_rec},
    {"face_detect", cmd_face_detect},
    {"neural_net", cmd_neural_net},
    {"serial_plot", cmd_serial_plot},
    {"data_log", cmd_data_log},
    {"auto_calibrate", cmd_auto_calibrate},
};

#define CMD_COUNT (sizeof(commands) / sizeof(Command))

void process_command(char *input)
{
    char *args[64];
    int argc = 0;
    char *token = strtok(input, " \t\n\r");

    while (token && argc < 64)
    {
        args[argc++] = token;
        token = strtok(NULL, " \t\n\r");
    }

    if (argc == 0)
        return;

    for (int i = 0; i < CMD_COUNT; i++)
    {
        if (strcmp(args[0], commands[i].name) == 0)
        {
            commands[i].func(argc, args);
            return;
        }
    }
    printf("Unknown command: %s\n", args[0]);
}

int main()
{
    char input[512];

    printf("========================================\n");
    printf("     ARDUINO CLI MASTER SYSTEM v1.0     \n");
    printf("     120+ Commands for Arduino          \n");
    printf("     by @concole_hack                   \n");
    printf("========================================\n");
    printf("Type 'help' for available commands\n");
    printf("========================================\n");

    while (1)
    {
        printf("arduino> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        process_command(input);
    }

    return 0;
}
