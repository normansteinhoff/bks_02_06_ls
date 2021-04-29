
// --------------------------------------------------------------------------------------

// Autor:
    // Norman Steinhoff

// Jahr
    // 2021

// --------------------------------------------------------------------------------------

// Verwendete Abkürzungen:
    // Vor. == Voraussetzung
    // Eff. == Effekt
    // Erg. == Ergebnis
    // Anm. == Anmerkung

// --------------------------------------------------------------------------------------

// neueren POSIX_Standard verwenden
#define _POSIX_C_SOURCE 200809L

// Standard_Header einbinden
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define FAILURE 1
#define SUCCESS 0
#define TOTAL_OPT_COUNT 2

// Vor.:
// *    { argc } und { argv } sind die selben wie in der Funktion { main() }.
// *    { opt_settings } hat genausoviele Einträge wie { strlen(opt_string) }
// Eff.:
// *    Wenn der Buchstabe an Stelle { i } des Strings { opt_string } als
//      optionales Argument übergeben wurde, dann ist im Array { opt_settings } der
//      Eintrag an der Stelle { i } gleich { true }.
// *    Ansonsten ist der Eintrag im Array { opt_settings } gleich { false }.
// Erg.:
// *    Der Betrag des Ergebnisses gibt die Anzahl der gefundenen gültigen optionalen Argumente an.
// *    Wenn alle gefundenen optionalen Argumente gültig waren ist das Ergebnis poitiv.
// *    Wenn mindestens ein gefundenes optionales Argument nicht gültig war ist das Ergebnis negativ.
// Anm.:
// *    { SUCCES } und { FAILURE } sind Makro_Konstanten.
int set_opt_array(int argc, char *argv[], char * opt_string, bool * opt_settings) {

    // Ergebnis initialisieren
    int result = 0;

    // Fehlerstatus initialisieren
    bool err = false;

    // optchar initialisieren
    int opt_char = getopt(argc, argv, opt_string);

    // Anzahl der gültigen optchars ermitteln
    unsigned long opt_valid_count = strlen(opt_string);

    // opt_settings initialisieren
    for (unsigned long a = 0; a < opt_valid_count; ++a) {

        // Markierung setzen
        opt_settings[a] = false;
    }

    // Über alle vorhandenen optchars iterieren
    while (opt_char != -1)
    {
        // Fehler gefunden
        if ('?' == opt_char) {

            // Fehler registrieren
            err = true;
        }

        // über alle gültigen optchars iterieren
        for (unsigned long a = 0; a < opt_valid_count; ++a) {

            // Prüfen, ob Übereinstimmung gefunden
            if (opt_string[a] == opt_char) {

                // Markierung setzen
                opt_settings[a] = true;

                // Ergebnis anpassen
                ++result; 

                // Schleife über gültige optchars abbrechen
                break;
            }
        }

        // nächsten optchar suchen
        opt_char = getopt(argc, argv, opt_string);
    }

    // Vorzeichen des Ergebnisses anpassen
    if (true == err) {

        // Ergebnis anpassen
        result = -result;
    }

    // Ergebnis liefern
    return result;
}

// Vor.:
// *    { argc } und { argv } sind die selben wie in der Funktion { main() }.
// Eff.:
// -
// Erg.:
// *    Die Anzahl der nicht_optionalen Argumente ist geliefert.
// Anm.:
// -
int count_non_opt_args(int argc, char *argv[]) {

    // Ergebnis initialisieren
    int result = 0;

    // nicht_optionale Argumente zählen
    for (int a = 1; a < argc; ++a) {

        // Prüfen, ob nicht_optionales Argument gefunden
        if (argv[a][0] != '-') { //gefunden

            // Ergebnis anpassen
            ++result;
        }
    }

    // Ergebnis liefern
    return result;
}

// Vor.:
//  
// Eff.:
// *    Ein String wird erzeugt. Dieser enthält den Zeit_Stempel von { namelist_entry }.
// Erg.:
// *    Ein Zeiger auf den erzeugten String ist geliefert.
// Anm.:
// *    Der String muss später mit { free() } entfernt werden.
char * get_time_string(struct dirent *namelist_entry) {

    // Ergebnis initialisieren
    char * result = NULL;

    // hier werden die Infos der Datei gespeichert
    struct stat buffer;

    // Datei_Infos ermitteln
    fstatat(
        AT_FDCWD, //spezielle Konstante, welche das Verhalten von fstatat beeinflusst
        namelist_entry->d_name, //path
        &buffer, //hier findet man anschließend die Infos zur Datei
        AT_SYMLINK_NOFOLLOW // bei einem Links zeige Infos über diesen, statt über die verknüpfte Datei
    );

    // Zeit_Stempel auslesen und in Bestandteile zerlegen (Jahr, Monat, Tag, Stunde, ...)
    struct tm * tm_ptr = localtime(&(buffer.st_ctime));
    
    // zerlegten Zeit_Stempel in String umwandeln
    result = asctime(tm_ptr);

    // Prüfen ob Ergebnis_String existiert
    if (NULL != result) { // existiert

        // Zeilenumbruch im String entfernen
        for (unsigned long a = 0; a < strlen(result); ++a) {

            // Prüfen, ob Zeilenumbruch
            if ('\n' == result[a]) {

                // Zeilenumbruch durch String_Ende ersetzen ersetzen
                result[a] = '\0';
            }
        }
    }
                
    // Ergebnis liefern
    return result;
}

// Vor.:
// 
// Eff.:
// *    Die Dateien im Verzeichnis { directory_name } werden angezeigt.
// *    Wenn { true == as_list } ist, erfolgt die Ausgabe als Liste.
// *    Wenn { true == with_hidden_files } ist, werden auch versteckte Dateien angezeigt.
// Erg.:
// -
// Anm.:
// -
void print_directories(char directory_name[], bool as_list, bool with_hidden_files) {

    // Pointer anlegen
    struct dirent **namelist = NULL;

    // Verzeichnis_Einträge ermitteln, in { namelist } speichern und alphabetisch sortieren
    int dir_count = scandir(directory_name, &namelist, NULL, alphasort);
    
    // Prüfen, ob Scan erfolgreich war
    if (dir_count < 0) {

        // Fehlermeldung
        printf("  Directory doesn't exist!\n");
    }
    else {

        //
        char * time_string;

        // Über Verzeichnis_Einträge iterieren
        for (int a = 0; a < dir_count; ++a) {

            //
            if ((with_hidden_files) || ((NULL != namelist[a]) && ('.' != namelist[a]->d_name[0]))){

                //
                if (as_list) {

                        //
                        time_string = get_time_string(namelist[a]);

                        // Eintrag ausgeben
                        printf("  %s %s\n", time_string, namelist[a]->d_name);
                }
                else {

                    //
                    printf("  %s\t", namelist[a]->d_name);
                }
            }

            // Prüfen ob { namelist[a] } existiert
            if (NULL != namelist[a]) {

                // Speicher freigeben
                free(namelist[a]);
            }
        }

        // abschließender Zeilenumbruch, falls nötig
        if (false == as_list) {
            
            //
            printf("\n");
        }
    }

    // Prüfen ob { namelist } existiert
    if (NULL != namelist) { // existiert

        // Speicher freigeben
        free(namelist);
    }

}

// Vor.:
// -
// Eff.:
// *    
// Erg.:
// *    Wenn ein Fehler auftrat, ist { FAILURE } geliefert.
// *    Ansonsten ist { SUCCCESS } geliefert.
// Anm.:
// *    { SUCCES } und { FAILURE } sind Makro_Konstanten.
int main(int argc, char *argv[]) {

    // Ergebnis initialisieren
    int result = SUCCESS;

    // gültige optionale Argumente festlegen
    char opt_string[TOTAL_OPT_COUNT] = "la";

    // Array für gefundene optionale Argumente vorbereiten
    bool opt_settings[TOTAL_OPT_COUNT];

    // Check auf optionale Argumente
    int opt_count = set_opt_array(argc, argv, opt_string, opt_settings);

    // Prüfen, ob ungültiges optionales Argument aufgetreten ist
    if (opt_count < 0) {

        // Ergebnis anpassen
        result = FAILURE;
    }

    // nicht_optionale Argumente zählen
    // Anm.:
    // *    { -la } sind zwar zwei optionale Argument, jedoch nehmen diese nur
    //      einen Platz in { argv } ein, im Gegensatz zu { -l -a}.
    //      Daher kann man nicht einfach { argc - opt_count } rechnen.
    int path_count = count_non_opt_args(argc, argv);

    // Prüfen, ob kein Pfad angegeben wurde
    if (path_count < 1) { // kein Pfad vorhanden

        // Ausgabe für aktuelles Verzeichnis
        print_directories(".", opt_settings[0], opt_settings[1]);
    }
    else { // mindestens ein Pfad vorhanden

        // alle Argumente durchgehen
        for (int a = 1; a < argc; ++a) {

            // Prüfen, ob Pfad gefunden
            if (argv[a][0] != '-') { // Pfad gefunden

                // Pfadname anzeigen
                printf("[%s]\n", argv[a]);

                // Ausgabe der Verzeichnis_Einträge des jeweiligen Pfades
                print_directories(argv[a], opt_settings[0], opt_settings[1]);
            }
        }
    }

    // Ergebnis liefern
    return result;
}
