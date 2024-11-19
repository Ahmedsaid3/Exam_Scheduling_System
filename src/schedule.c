#include "schedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Create a new schedule with 7 days
struct Schedule* CreateSchedule() {
    // day'leri olusturma ve birbirlerine baglama
    struct Day* monday = (struct Day*) malloc(sizeof(struct Day));
    strcpy(monday->dayName, "Monday");
    monday->examList = NULL;

    struct Day* tuesday = (struct Day*) malloc(sizeof(struct Day));
    strcpy(tuesday->dayName, "Tuesday");
    tuesday->examList = NULL;
    monday->nextDay = tuesday;

    struct Day* wednesday = (struct Day*) malloc(sizeof(struct Day));
    strcpy(wednesday->dayName, "Wednesday");
    wednesday->examList = NULL;
    tuesday->nextDay = wednesday;

    struct Day* thursday = (struct Day*) malloc(sizeof(struct Day));
    strcpy(thursday->dayName, "Thursday");
    thursday->examList = NULL;
    wednesday->nextDay = thursday;

    struct Day* friday = (struct Day*) malloc(sizeof(struct Day));
    strcpy(friday->dayName, "Friday");
    friday->examList = NULL;
    thursday->nextDay = friday;

    struct Day* saturday = (struct Day*) malloc(sizeof(struct Day));
    strcpy(saturday->dayName, "Saturday");
    saturday->examList = NULL;
    friday->nextDay = saturday;

    struct Day* sunday = (struct Day*) malloc(sizeof(struct Day));
    strcpy(sunday->dayName, "Sunday");
    sunday->examList = NULL;
    saturday->nextDay = sunday;

    sunday->nextDay = monday;

    // schedule olusturma ve mondayi atama 
    struct Schedule* schedule = (struct Schedule*) malloc(sizeof(struct Schedule));
    schedule->head = monday;

    printf("Schedule creation complete.\n");

    return schedule;
}

// Add an exam to a day in the schedule
int AddExamToSchedule(struct Schedule* schedule, const char* day, int startTime, int endTime, const char* courseCode) {

    // return 3 eger exam suresi 3 ten buyukse, veya start ve end time allowed araligin disindaysa
    if (endTime - startTime > 3 || startTime < 8 || startTime > 17 || endTime < 9 || endTime > 20) {
        printf("Invalid exam.\n");
        return 3; 
    }

    // once istenen gunu bulmam lazim: 
    struct Day* tmp = schedule->head;
    while (tmp->nextDay != schedule->head) {
        if (strcmp(tmp->dayName, day) == 0) { // strcmp eger iki string ayni ise 0 donduren bi fonksiyon 
            break; // donguden cikicam, istenen gunu bulmus oldum, tmp istenen gunu gosteriyor
        }

        tmp = tmp->nextDay;
    }


    // return 0 eger istenen gun ve saate eklendiyse 
    if (tmp->examList == NULL) { // hic sinav yokken
        struct Exam* exam = CreateExam(startTime, endTime, courseCode);
        tmp->examList = exam;
        printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);
        return 0;
    } else if (tmp->examList->next == NULL) { // 1 sinav varken 2.yi ekleme durumu
        if (startTime >= tmp->examList->endTime) {
            struct Exam* exam = CreateExam(startTime, endTime, courseCode);
            printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);
            tmp->examList->next = exam;
            return 0;
        
        } else if (endTime <= tmp->examList->startTime) { // bu durumda tmp->examList degismeli, cunku head'i yani en bastakini gostermeli 
            struct Exam* exam = CreateExam(startTime, endTime, courseCode);
            printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);
            struct Exam* cpy = tmp->examList;
            tmp->examList = exam;
            tmp->examList->next = cpy;
            return 0;  
        }
    } else { // 2 sinav var, 3.yu veya daha fazlasini ekleme -> 3 durum var, basa araya ve sona ekleme, buna gore temp->examList degisecek

        // 1.durum -> basa ekleme 
        if (endTime <= tmp->examList->startTime) { 
            struct Exam* exam = CreateExam(startTime, endTime, courseCode);
            printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
            exam->next = tmp->examList; // derste yaptigimiz add to head mantigi aslinda, cpy olusturmaya gerek yok
            tmp->examList = exam;
            return 0;          
        }

        // son exam nodu'nu elde etme (2. ve 3.durumda kullandim): 
        struct Exam* cpy = tmp->examList;
        while (cpy->next != NULL) { // cpy bu dongu sonunda son exam node'una point ediyor
            cpy = cpy->next;
        }

        // 2.durum -> araya ekleme (prev tutarak ilerlicem):
        if (startTime >= tmp->examList->endTime && endTime <= cpy->startTime) {
            struct Exam* exam = tmp->examList;
            struct Exam* prev = NULL;
            while (exam != NULL) {

                if (prev != NULL && startTime >= prev->endTime && endTime <= exam->startTime) { // prev != NULL koydum ilk basta buraya bakip hata vermesin diye
                struct Exam* newExam = CreateExam(startTime, endTime, courseCode);
                printf("%s exam added to %s at time %d to %d without conflict.\n", newExam->courseCode, tmp->dayName, newExam->startTime, newExam->endTime);
                newExam->next = exam;
                prev->next = newExam;
                return 0;
                }

                prev = exam;
                exam = exam->next;
            }

        }

        // 3.durum -> sona ekleme
        if (startTime >= cpy->endTime) {
            struct Exam* exam = CreateExam(startTime, endTime, courseCode);
            printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
            cpy->next = exam; // son node'u yeni node'a bagladim
            return 0; 
        }

    } // return 0 bitisi 


    // return 1 eger sonraki bosluklara eklendiyse (ayni gun veya sonraki gunler, nearest possible time)
    if (tmp->examList->next == NULL) { // 1 sinav var, overlap etmis 2.dersi ekleme durumu

        if ((startTime >= tmp->examList->startTime && startTime <= tmp->examList->endTime) ||
            (endTime >= tmp->examList->startTime && endTime <= tmp->examList->endTime) || 
            (startTime < tmp->examList->startTime && endTime > tmp->examList->endTime)) { // overlap condition

                int duration = endTime - startTime; 
                int newStartTime = tmp->examList->endTime;
                int newEndTime = tmp->examList->endTime + duration; 

                if (newEndTime <= 20) { 
                    struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                    printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
                    tmp->examList->next = exam;
                    return 1; 
                } else { // sonraki gune gecme durumu  
                    // struct Day* curDay = tmp; // bulundugu gune kadar degil de sinava kadar yaptim
                    newStartTime = 8;
                    newEndTime = 8 + duration;

                    struct Exam* sinav = tmp->examList; // 1 sinav vardi zaten ekledigim gun 
                    struct Exam* cpy = NULL;

                    while (cpy != sinav) { // tmp->nextDay != curDay idi burasi, bulundugu gune kadar bakma durumunda dogru calisiyodu
                        tmp = tmp->nextDay; // bi sonraki gune gectim

                        cpy = tmp->examList;
                        struct Exam* prev = NULL;

                        if (cpy == NULL || cpy->startTime >= newEndTime) { // sonraki gunde sinav yoksa veya 8-(8+duration) araliginda yoksa, bu durumda tmp->examList degisir 
                            struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                            printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  

                            struct Exam* tmp2 = tmp->examList;
                            tmp->examList = exam;
                            exam->next = tmp2;
                            return 1;
                        } else {

                            while (cpy != NULL) { 
                                
                                if ((newStartTime >= cpy->startTime && newStartTime < cpy->endTime) ||
                                    (newEndTime > cpy->startTime && newEndTime <= cpy->endTime) || 
                                    (newStartTime < cpy->startTime && newEndTime > cpy->endTime)) { // overlap condition (dogru olan)

                                        newStartTime = cpy->endTime;
                                        newEndTime = cpy->endTime + duration; 

                                        prev = cpy;
                                        cpy = cpy->next; 
                                        if (cpy == NULL && newEndTime <= 20) { // sona ekleme islemi 
                                            struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                                            printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);                                             
                                            prev->next = exam;
                                            return 1;
                                        } else if (cpy == NULL && newEndTime > 20) { // sonraki gun 
                                            newStartTime = 8;
                                            newEndTime = 8 + duration;
                                            break;
                                        }
                                    } else { // araya ekleme mantigi
                                        struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                                        printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  

                                        exam->next = cpy; 
                                        prev->next = exam;
                                        return 1;
                                    }

                            } 
                        }

                    } // gunleri dolastigim while'in sonu, bulundugum gune geri geldiysem yani schedule doluysa
                    printf("\nSchedule full. Exam cannot be added.\n");
                    return 2;
                } 

        }
    } else if (tmp->examList->next != NULL) { // 2 veya daha fazla sinav var, overlap eden yeni dersi ekleme durumu

        // son exam nodu'nu elde etme
        struct Exam* last = tmp->examList;
        while (last->next != NULL) { 
            last = last->next;
        }

        
        struct Exam* cpy = tmp->examList; 
        int duration = endTime - startTime;
        int newStartTime = startTime;
        int newEndTime = endTime;

        while (cpy != NULL) { 

            // araya ekleme durumu: 
            if (cpy != last && 
            ((newStartTime >= cpy->startTime && newStartTime < cpy->endTime) || 
            (newEndTime > cpy->startTime && newEndTime <= cpy->endTime) || 
            (newStartTime < cpy->startTime && newEndTime > cpy->endTime))) {

                newStartTime = cpy->endTime;
                newEndTime = cpy->endTime + duration;

                if (newEndTime <= cpy->next->startTime) { // eger bi sonraki bosluk uygunsa 
                    struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                    printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
                    exam->next = cpy->next;
                    cpy->next = exam;
                    return 1; 
                } else { // sonraki bosluk uygun degilse

                    cpy = cpy->next; 
                    continue;  
                }
            } // sona ekleme durumu: 
            else if (cpy == last &&
            ((newStartTime >= cpy->startTime && newStartTime < cpy->endTime) || 
            (newEndTime > cpy->startTime && newEndTime <= cpy->endTime) || 
            (newStartTime < cpy->startTime && newEndTime > cpy->endTime))) {

                newStartTime = cpy->endTime;
                newEndTime = cpy->endTime + duration;
                if (newEndTime <= 20) { 
                    struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                    printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
                    last->next = exam;
                    return 1; 
                } else { // sonraki gune gecme  
                    newStartTime = 8;
                    newEndTime = 8 + duration;

                    struct Exam* sinav = cpy;  
                    struct Exam* cpy2 = NULL;
                    struct Exam* prev = NULL;

                    while (prev != sinav) { // tmp->nextDay != curDay idi burasi, bulundugu gune kadar bakma durumunda dogru calisiyodu
                        tmp = tmp->nextDay; // bi sonraki gune gectim

                        cpy2 = tmp->examList;
                        prev = NULL;

                        if (cpy2 == NULL || cpy2->startTime >= newEndTime) { // sonraki gunde sinav yoksa veya 8-(8+duration) araliginda yoksa, bu durumda tmp->examList degisir 
                            struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                            printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  

                            struct Exam* tmp2 = tmp->examList;
                            tmp->examList = exam;
                            exam->next = tmp2;
                            return 1;
                        } else {

                            while (cpy2 != NULL) { 
                                
                                if ((newStartTime >= cpy2->startTime && newStartTime < cpy2->endTime) ||
                                    (newEndTime > cpy2->startTime && newEndTime <= cpy2->endTime) || 
                                    (newStartTime < cpy2->startTime && newEndTime > cpy2->endTime)) { // overlap condition (dogru olan)

                                        newStartTime = cpy2->endTime;
                                        newEndTime = cpy2->endTime + duration; 

                                        prev = cpy2;
                                        cpy2 = cpy2->next; 
                                        if (cpy2 == NULL && newEndTime <= 20) { // sona ekleme islemi 
                                            struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                                            printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);                                             
                                            prev->next = exam;
                                            return 1;
                                        } else if (cpy2 == NULL && newEndTime > 20) { // sonraki gun 
                                            newStartTime = 8;
                                            newEndTime = 8 + duration;
                                            break;
                                        }
                                    } else { // araya ekleme mantigi
                                        struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                                        printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  

                                        exam->next = cpy2; 
                                        prev->next = exam;
                                        return 1;
                                    }

                            } 
                        }

                    } // gunleri dolastigim while'in sonu, bulundugum gune geri geldiysem yani schedule doluysa
                    printf("\nSchedule full. Exam cannot be added.\n");
                    return 2;
                }
            }

            cpy = cpy->next;

        }
    }

    return -1; // compiler hata vermesin diye ekledim

} // add_to_exam bitisi

// clear edilmek istenen gune kadar kontrol edip eklememi saglayan helper function
int AddExam2ToSchedule(struct Schedule* schedule, const char* day, int startTime, int endTime, const char* courseCode, const char* day2) {

    // ! bu fonksiyonda return 3'u kaldirdim, cunku clear sinavlardan birini 18-20 araligina ekledikten sonra 3 saatligi clear ederken 21 olur mesela, return 3 olmadan overlap olup diger gunden baslamali eklemeye
    // return 3 eger exam suresi 3 ten buyukse, veya start ve end time allowed araligin disindaysa
    // if (endTime - startTime > 3 || startTime < 8 || startTime > 17 || endTime < 9 || endTime > 20) {
    //     printf("Invalid exam.\n");
    //     return 3; 
    // }

    // once istenen gunu bulmam lazim: 
    struct Day* tmp = schedule->head;
    while (tmp->nextDay != schedule->head) {
        if (strcmp(tmp->dayName, day) == 0) { // strcmp eger iki string ayni ise 0 donduren bi fonksiyon 
            break; // donguden cikicam, istenen gunu bulmus oldum, tmp istenen gunu gosteriyor
        }

        tmp = tmp->nextDay;
    }


    // return 0 eger istenen gun ve saate eklendiyse 
    if (tmp->examList == NULL) { // hic sinav yokken
        struct Exam* exam = CreateExam(startTime, endTime, courseCode);
        tmp->examList = exam;
        printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);
        return 0;
    } else if (tmp->examList->next == NULL) { // 1 sinav varken 2.yi ekleme durumu
        if (startTime >= tmp->examList->endTime) {
            struct Exam* exam = CreateExam(startTime, endTime, courseCode);
            printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);
            tmp->examList->next = exam;
            return 0;
        
        } else if (endTime <= tmp->examList->startTime) { // bu durumda tmp->examList degismeli, cunku head'i yani en bastakini gostermeli 
            struct Exam* exam = CreateExam(startTime, endTime, courseCode);
            printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);
            struct Exam* cpy = tmp->examList;
            tmp->examList = exam;
            tmp->examList->next = cpy;
            return 0;  
        }
    } else { // 2 sinav var, 3.yu veya daha fazlasini ekleme -> 3 durum var, basa araya ve sona ekleme, buna gore temp->examList degisecek

        // 1.durum -> basa ekleme 
        if (endTime <= tmp->examList->startTime) { 
            struct Exam* exam = CreateExam(startTime, endTime, courseCode);
            printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
            exam->next = tmp->examList; // derste yaptigimiz add to head mantigi aslinda, cpy olusturmaya gerek yok
            tmp->examList = exam;
            return 0;          
        }

        // son exam nodu'nu elde etme (2. ve 3.durumda kullandim): 
        struct Exam* cpy = tmp->examList;
        while (cpy->next != NULL) { // cpy bu dongu sonunda son exam node'una point ediyor
            cpy = cpy->next;
        }

        // 2.durum -> araya ekleme (prev tutarak ilerlicem):
        if (startTime >= tmp->examList->endTime && endTime <= cpy->startTime) {
            struct Exam* exam = tmp->examList;
            struct Exam* prev = NULL;
            while (exam != NULL) {

                if (prev != NULL && startTime >= prev->endTime && endTime <= exam->startTime) { // prev != NULL koydum ilk basta buraya bakip hata vermesin diye
                struct Exam* newExam = CreateExam(startTime, endTime, courseCode);
                printf("%s exam added to %s at time %d to %d without conflict.\n", newExam->courseCode, tmp->dayName, newExam->startTime, newExam->endTime);
                newExam->next = exam;
                prev->next = newExam;
                return 0;
                }

                prev = exam;
                exam = exam->next;
            }

        }

        // 3.durum -> sona ekleme
        if (startTime >= cpy->endTime) {
            struct Exam* exam = CreateExam(startTime, endTime, courseCode);
            printf("%s exam added to %s at time %d to %d without conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
            cpy->next = exam; // son node'u yeni node'a bagladim
            return 0; 
        }

    } // return 0 bitisi 


    // return 1 eger sonraki bosluklara eklendiyse (ayni gun veya sonraki gunler, nearest possible time)
    if (tmp->examList->next == NULL) { // 1 sinav var, overlap etmis 2.dersi ekleme durumu

        if ((startTime >= tmp->examList->startTime && startTime <= tmp->examList->endTime) ||
            (endTime >= tmp->examList->startTime && endTime <= tmp->examList->endTime) || 
            (startTime < tmp->examList->startTime && endTime > tmp->examList->endTime)) { // overlap condition

                int duration = endTime - startTime; 
                int newStartTime = tmp->examList->endTime;
                int newEndTime = tmp->examList->endTime + duration; 

                if (newEndTime <= 20) { 
                    struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                    printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
                    tmp->examList->next = exam;
                    return 1; 
                } else { // sonraki gune gecme durumu  
                    // struct Day* curDay = tmp; // bulundugu gune kadar degil de sinava kadar yaptim
                    newStartTime = 8;
                    newEndTime = 8 + duration;

                    struct Exam* sinav = tmp->examList; // 1 sinav vardi zaten ekledigim gun 
                    struct Exam* cpy = NULL;

                    while (strcmp(tmp->nextDay->dayName, day2) != 0) { // ! burayi clear etmek istedigim gune kadar olarak degistirdim
                        tmp = tmp->nextDay; // bi sonraki gune gectim

                        cpy = tmp->examList;
                        struct Exam* prev = NULL;

                        if (cpy == NULL || cpy->startTime >= newEndTime) { // sonraki gunde sinav yoksa veya 8-(8+duration) araliginda yoksa, bu durumda tmp->examList degisir 
                            struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                            printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  

                            struct Exam* tmp2 = tmp->examList;
                            tmp->examList = exam;
                            exam->next = tmp2;
                            return 1;
                        } else {

                            while (cpy != NULL) { 
                                
                                if ((newStartTime >= cpy->startTime && newStartTime < cpy->endTime) ||
                                    (newEndTime > cpy->startTime && newEndTime <= cpy->endTime) || 
                                    (newStartTime < cpy->startTime && newEndTime > cpy->endTime)) { // overlap condition (dogru olan)

                                        newStartTime = cpy->endTime;
                                        newEndTime = cpy->endTime + duration; 

                                        prev = cpy;
                                        cpy = cpy->next; 
                                        if (cpy == NULL && newEndTime <= 20) { // sona ekleme islemi 
                                            struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                                            printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);                                             
                                            prev->next = exam;
                                            return 1;
                                        } else if (cpy == NULL && newEndTime > 20) { // sonraki gun 
                                            newStartTime = 8;
                                            newEndTime = 8 + duration;
                                            break;
                                        }
                                    } else { // araya ekleme mantigi
                                        struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                                        printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  

                                        exam->next = cpy; 
                                        prev->next = exam;
                                        return 1;
                                    }

                            } 
                        }

                    } // gunleri dolastigim while'in sonu, bulundugum gune geri geldiysem yani schedule doluysa
                    printf("\nSchedule full. Exam cannot be added.\n");
                    return 2;
                } 

        }
    } else if (tmp->examList->next != NULL) { // 2 veya daha fazla sinav var, overlap eden yeni dersi ekleme durumu

        // son exam nodu'nu elde etme
        struct Exam* last = tmp->examList;
        while (last->next != NULL) { 
            last = last->next;
        }

        
        struct Exam* cpy = tmp->examList; 
        int duration = endTime - startTime;
        int newStartTime = startTime;
        int newEndTime = endTime;

        while (cpy != NULL) { 

            // araya ekleme durumu: 
            if (cpy != last && 
            ((newStartTime >= cpy->startTime && newStartTime < cpy->endTime) || 
            (newEndTime > cpy->startTime && newEndTime <= cpy->endTime) || 
            (newStartTime < cpy->startTime && newEndTime > cpy->endTime))) {

                newStartTime = cpy->endTime;
                newEndTime = cpy->endTime + duration;

                if (newEndTime <= cpy->next->startTime) { // eger bi sonraki bosluk uygunsa 
                    struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                    printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
                    exam->next = cpy->next;
                    cpy->next = exam;
                    return 1; 
                } else { // sonraki bosluk uygun degilse

                    cpy = cpy->next; 
                    continue;  
                }
            } // sona ekleme durumu: 
            else if (cpy == last &&
            ((newStartTime >= cpy->startTime && newStartTime < cpy->endTime) || 
            (newEndTime > cpy->startTime && newEndTime <= cpy->endTime) || 
            (newStartTime < cpy->startTime && newEndTime > cpy->endTime))) {

                newStartTime = cpy->endTime;
                newEndTime = cpy->endTime + duration;
                if (newEndTime <= 20) { 
                    struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                    printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  
                    last->next = exam;
                    return 1; 
                } else { // sonraki gune gecme  
                    newStartTime = 8;
                    newEndTime = 8 + duration;

                    struct Exam* sinav = cpy;  
                    struct Exam* cpy2 = NULL;
                    struct Exam* prev = NULL;

                    while (strcmp(tmp->nextDay->dayName, day2) != 0) { // ! burayi clear etmek istedigim gune kadar olarak degistirdim
                        tmp = tmp->nextDay; // bi sonraki gune gectim

                        cpy2 = tmp->examList;
                        prev = NULL;

                        if (cpy2 == NULL || cpy2->startTime >= newEndTime) { // sonraki gunde sinav yoksa veya 8-(8+duration) araliginda yoksa, bu durumda tmp->examList degisir 
                            struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                            printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  

                            struct Exam* tmp2 = tmp->examList;
                            tmp->examList = exam;
                            exam->next = tmp2;
                            return 1;
                        } else {

                            while (cpy2 != NULL) { 
                                
                                if ((newStartTime >= cpy2->startTime && newStartTime < cpy2->endTime) ||
                                    (newEndTime > cpy2->startTime && newEndTime <= cpy2->endTime) || 
                                    (newStartTime < cpy2->startTime && newEndTime > cpy2->endTime)) { // overlap condition (dogru olan)

                                        newStartTime = cpy2->endTime;
                                        newEndTime = cpy2->endTime + duration; 

                                        prev = cpy2;
                                        cpy2 = cpy2->next; 
                                        if (cpy2 == NULL && newEndTime <= 20) { // sona ekleme islemi 
                                            struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                                            printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);                                             
                                            prev->next = exam;
                                            return 1;
                                        } else if (cpy2 == NULL && newEndTime > 20) { // sonraki gun 
                                            newStartTime = 8;
                                            newEndTime = 8 + duration;
                                            break;
                                        }
                                    } else { // araya ekleme mantigi
                                        struct Exam* exam = CreateExam(newStartTime, newEndTime, courseCode);
                                        printf("%s exam added to %s at time %d to %d due to conflict.\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);  

                                        exam->next = cpy2; 
                                        prev->next = exam;
                                        return 1;
                                    }

                            } 
                        }

                    } // gunleri dolastigim while'in sonu, bulundugum gune geri geldiysem yani schedule doluysa
                    printf("\nSchedule full. Exam cannot be added.\n");
                    return 2;
                }
            }

            cpy = cpy->next;

        }
    }

    return -1; // compiler hata vermesin diye ekledim

} // add_to_exam bitisi

// Remove an exam from a specific day in the schedule
int RemoveExamFromSchedule(struct Schedule* schedule, const char* day, int startTime) {
    // istenen gunu bulma
    struct Day* tmp = schedule->head;
    while (tmp->nextDay != schedule->head) {
        if (strcmp(tmp->dayName, day) == 0) {  
            break; // donguden cikicam, istenen gunu bulmus oldum, tmp istenen gunu gosteriyor
        }

        tmp = tmp->nextDay;
    }

    struct Exam* exam = tmp->examList;
    struct Exam* prev = NULL;

    while (exam != NULL) {

        if (exam->startTime == startTime) {

            if (exam == tmp->examList) { // en bastaki eleman ise tmp->examList degismeli
                tmp->examList = exam->next;
                free(exam);
                printf("Exam removed successfully.\n");
                return 0;
            }

            prev->next = exam->next;
            free(exam);
            printf("Exam removed successfully.\n");
            return 0;
            
        }

        prev = exam;
        exam = exam->next;
    }

    printf("Exam could not be found.\n");
    return 1;

}

// Update an exam in the schedule
int UpdateExam(struct Schedule* schedule, const char* oldDay, int oldStartTime, const char* newDay, int newStartTime, int newEndTime) {

    // return 3 eger yeni exam suresi 3 ten buyukse, veya start ve end time allowed araligin disindaysa
    if (newEndTime - newStartTime > 3 || newStartTime < 8 || newStartTime > 17 || newEndTime < 9 || newEndTime > 20) {
        printf("Invalid exam.\n");
        return 3; 
    }

    // istenen gunu bulma
    struct Day* tmp = schedule->head;
    while (tmp->nextDay != schedule->head) {
        if (strcmp(tmp->dayName, oldDay) == 0) {  
            break; // donguden cikicam, istenen gunu bulmus oldum, tmp istenen gunu gosteriyor
        }

        tmp = tmp->nextDay;
    }

    // sinav var mi kontrol etme
    struct Exam* exam = tmp->examList;

    while (exam != NULL) {
        if (exam->startTime == oldStartTime) {
            
            int duration = exam->endTime - exam->startTime;
            char courseCode[50];
            strcpy(courseCode, exam->courseCode); 

            int result = AddExamToSchedule(schedule, newDay, newStartTime, newEndTime, courseCode);
            if (result == 0) {
                RemoveExamFromSchedule(schedule, oldDay, oldStartTime);
                printf("Update successful.\n");
                return 0;
            } else if (result == 1) {
                RemoveExamFromSchedule(schedule, oldDay, oldStartTime); // eskiyi yolladim, tekrar eklicem, cunku update olmadi
                // conflict oldu ve gereksiz yere yeni bi sinav ekledi uygun buldugu bi bosluga, courseCode'dan yerini bulup remove'licam onu
                struct Day* day = tmp;
                struct Exam* cpy = tmp->examList;

                while (day != NULL) {  
                    cpy = day->examList;

                    while (cpy != NULL) {

                        if (strcmp(cpy->courseCode, courseCode) == 0) {
                            RemoveExamFromSchedule(schedule, day->dayName, cpy->startTime);
                            AddExamToSchedule(schedule, oldDay, oldStartTime, oldStartTime+duration, courseCode); // attigim yere geri koydum
                            printf("Update unsuccessful.\n");
                            return 1;
                        }

                        cpy = cpy->next;
                    }

                    day = day->nextDay;
                }
            } else if (result == 2) {
                printf("Update unsuccessful.\n");
                return 1;
            }

        }

        exam = exam->next;
    }

    // sinav yoksa: 
    printf("Exam could not be found.\n");
    return 2;

}

// Clear all exams from a specific day and relocate them to other days
int ClearDay(struct Schedule* schedule, const char* day) {
    // istenen gunu bulma:
    struct Day* tmp = schedule->head;
    while (tmp->nextDay != schedule->head) {
        if (strcmp(tmp->dayName, day) == 0) {  
            break; // donguden cikicam, istenen gunu bulmus oldum, tmp istenen gunu gosteriyor
        }

        tmp = tmp->nextDay;
    }

    if (tmp->examList == NULL) {
        printf("%s is already clear.\n", tmp->dayName);
        return 1;
    }

    struct Exam* exam = tmp->examList;
    struct Day* gun = tmp->nextDay;
    int duration = exam->endTime - exam->startTime;
    int newStartTime = 8;
    int newEndTime = 8 + duration;

    // result 2'ye girme durumunda kullanicam bunlari
    struct Day* removeDay = NULL; // result 0 icinde kullandim

    struct Day** removeDayArray = (struct Day**) malloc(10 * sizeof(struct Day*)); // result 1 icinde kullandim
    int removeStartTimeArray[15]; 
    int count = 0;

    while (exam != NULL) {

        char courseCode[50];
        strcpy(courseCode, exam->courseCode); 

        int result = AddExam2ToSchedule(schedule, gun->dayName, newStartTime, newEndTime, courseCode, day); // son parametre temizlemek istedigim gun, clear day'in 2.parametresi

        if (result == 0) { 
            removeDay = gun; 
            exam = exam->next;
            duration = exam->endTime - exam->startTime;
            newEndTime = newStartTime + duration; // newStartTime'i degistirmedim bi sonrakinde overlap olmasi icin, yani bu if'e bir defa giricek, bundan sonrasinda ekleyebildiyse result 1'e girecek
        }

        if (result == 1) { // burda gun, start end time degismeli 

            struct Day* tmp2 = tmp->nextDay;
            struct Exam* cpy = NULL;

            while (tmp2 != tmp) { // yani clear edilmek istenen gune kadar kontrol ediyorum course code'u  
                cpy = tmp2->examList;

                while (cpy != NULL) {

                    if (strcmp(cpy->courseCode, courseCode) == 0) { 
                        *(removeDayArray + count) = tmp2;
                        removeStartTimeArray[count] = cpy->startTime;
                        count++;

                        gun = tmp2;
                        exam = exam->next;
                        if (exam != NULL) { // bunu yapmazsam seg fault aliyorum
                            duration = exam->endTime - exam->startTime;  
                            newStartTime = cpy->startTime;
                            newEndTime = newStartTime + duration; 
                        }

                        break;
                    }

                    cpy = cpy->next;
                }

                tmp2 = tmp2->nextDay;
            }

        } else if (result == 2) { // clearday'dekilerden herhangi birini yerlestiremediyse buraya giriyor, eklenen course codelari bulup yok etme islemi yapmaliyim
            // ekleyip result 0'a girdiyse:
            if (removeDay != NULL) 
                RemoveExamFromSchedule(schedule, removeDay->dayName, 8); 

            // eklenip result 1'e girenler
            for (int i = 0; i < count; i++) {
                int remove_start_time = removeStartTimeArray[i];
                char remove_day_name[50];
                strcpy(remove_day_name, (*(removeDayArray + i))->dayName);
                RemoveExamFromSchedule(schedule, remove_day_name, remove_start_time);
            }
            
            printf("Schedule full. Exams from %s could not be relocated.\n", day);
            return 2;
        }
    }
    
    free(removeDayArray); // malloc yaptigim icin bunu free'lemem lazim

    // hepsi yerlesti kronolojik olarak, clear daydekileri removeladim 
    struct Exam* sinav = tmp->examList;
    struct Exam* prev = NULL;
    while (sinav != NULL) {
        prev = sinav;
        sinav = sinav->next;
        free(prev);
    }
    tmp->examList = NULL;

    printf("%s is cleared, exams relocated.\n", day);
    return 0;

}

// Clear all exams and days from the schedule and deallocate memory
void DeleteSchedule(struct Schedule* schedule) {
    struct Day* tmp = schedule->head;

    // sunday'i bulma (last):
    struct Day* cpy = schedule->head;
    while (schedule->head) { // true yazmak yerine bu mantigi kullaniyorum, NULL olmadigi hicbir zaman sonsuz dongu olusturabiliyorum 
        if (cpy->nextDay == schedule->head) {
            cpy->nextDay = NULL; // pazarin next'ini null yaptim, asagidaki while da kullandim
            break;
        }
        cpy = cpy->nextDay;
    }

    struct Day* tmp2 = NULL;
    struct Exam* exam = NULL;
    struct Exam* prev = NULL;

    while (tmp != NULL) {
        exam = tmp->examList;

        while (exam != NULL) { // sinavlari yok etme
            prev = exam;
            exam = exam->next;
            free(prev);
        }

        tmp2 = tmp; // gunleri yok etme
        tmp = tmp->nextDay;
        free(tmp2);
    }

    free(schedule);
    schedule->head = NULL; 

    printf("Schedule is cleared.\n");
}

// Read schedule from file
int ReadScheduleFromFile(struct Schedule* schedule, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Input File can not be opened!\n");
        return -1;
    }
    
    const char* days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

    // add fonksiyonuna vermek icin almam lazim olan bilgiler
    char day[50];
    int startTime;
    int endTime;
    char courseCode[50];
    int isDay;

    char line[50];
    while (fgets(line, sizeof(line), file)) { // fgets line by line okuyor, okudugu satiri line degiskenine aktariyor

        // strcmp'de hata almamak icin
        line[strcspn(line, "\n")] = '\0';

        // okudugum satir bos ise: 
        if (line[0] == '\0') {
            continue; 
        }

        // okudugum satir gun ise: 
        isDay = 0;
        for (int i = 0; i < 7; i++) {
            if (strcmp(line, days[i]) == 0) {
                strcpy(day, line);
                isDay = 1;
                break;
            }
        }

        // okudugum satir sinav bilgileri ise: 
        if (!isDay) {
            sscanf(line, "%d %d %s", &startTime, &endTime, courseCode); // sirasiyla degerleri atadi
            
            AddExamToSchedule(schedule, day, startTime, endTime, courseCode);
        }
    }

    fclose(file);
    return 0; 
}

// Write schedule to file
int WriteScheduleToFile(struct Schedule* schedule, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Output File can not be opened!\n");
        return -1;
    }

    struct Day* tmp = schedule->head;

    do {
        struct Exam* exam = tmp->examList;

        fprintf(file, "%s", tmp->dayName);
        fprintf(file, "\n");

        if (exam == NULL) {
            fprintf(file, "%s", "(No exams scheduled)");
            fprintf(file, "\n");
        }

        while (exam != NULL) {

            fprintf(file, "%d %d %s", exam->startTime, exam->endTime, exam->courseCode);
            fprintf(file, "\n");

            exam = exam->next;
        }

        fprintf(file, "\n");
        tmp = tmp->nextDay;
    } while (tmp != schedule->head);

    fclose(file);
    return 0;
}

// helper function (linked listleri kontrol etmek icin)
void traverseExamList(struct Schedule* schedule, const char* day) {
    printf("\nTraverse ");
    printf("%s:\n", day);

    // traverse edilmek istenen gunu bulmak icin
    struct Day* tmp = schedule->head;
    while (tmp->nextDay != schedule->head) {
        if (strcmp(tmp->dayName, day) == 0) {  
            break; 
        }

        tmp = tmp->nextDay;
    }

    struct Exam* exam = tmp->examList;
    while (exam != NULL) {
        printf("%s exam added to %s at time %d to %d\n", exam->courseCode, tmp->dayName, exam->startTime, exam->endTime);
        exam = exam->next;
    }
}

