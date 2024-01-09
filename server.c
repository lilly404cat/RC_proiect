#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <ctype.h>
#define PORT 8888

extern int errno;

MYSQL *con;

typedef struct {
    pthread_t idThread;
    int thCount;
} Thread;

int socket_desc, nrThreads;
Thread *threadPool;
pthread_mutex_t mlock=PTHREAD_MUTEX_INITIALIZER;

void raspunde(int cl, int idThread)
{
    int loginStatus = 0, logoutStatus = 1, isProv, chsdProv, flag = -1, isincart= 0, id_prov = -1, id_user = -1, iszero=-1;
    char user_name[255], message[2048];
    
    while(1)
    {
        int nr = 0;
        char nr_c;
        char query[1024];
        if(recv(cl, &nr, sizeof(int), 0) <= 0)
        {
            printf("[Thread %d]\n",idThread);
            perror ("Eroare la recv() de la client.\n");	
        }

        printf ("[Thread %d]Mesajul a fost receptionat...%d\n",idThread, nr);
        if(nr == 1 )
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                fflush(stdout);
                printf("[server] client %d want to sign up", idThread);
                nr++;   
                loginStatus = 1;
                logoutStatus = 0;
                char *data[6] ={"user_name", "password", "email", "phone", "location", "isProvider"};
                char info[255];
                printf("[Thread %d]Trimitem mesajul inapoi...%d\n",idThread, nr);
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                user_name[0]='\0';
                for( int i = 0; i < 6; ++i)
                {
                    info[0] = '\0';
                    message[0]='\0';
                    while(1)
                    {
                        if(recv(cl, info, sizeof(info), 0) <= 0) 
                        {
                            fprintf(stderr, "[client]Error reciving from  client %s\n", data[i]);
                        }
                        printf("%s", info);
                        info[strcspn(info, "\n")] = '\0';
                        if(i == 5)
                        {
                            if(strstr(info, "yes")) 
                                isProv = 1;
                            else
                                isProv = 0;
                            if (send(cl, &isProv, sizeof(int), 0) <= 0) 
                            {
                                printf("[Thread %d] ", idThread);
                                perror("[Thread]Eroare la send() catre client.\n");
                            }
                            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                            
                            snprintf(query, sizeof(query), "UPDATE users SET isProvider = %d where user_name like '%s';", isProv, user_name);
                            printf("%s", query);
                            if(mysql_query(con, query))
                            {
                                fprintf(stderr, "%s\n", mysql_error(con)); 
                            }
                            break;
                        }
                        if(strlen(info) > 1)
                        {
                            if(strstr(data[i], "user_name"))
                            {
                                strcpy(user_name, info);
                                snprintf(query, sizeof(query), "INSERT INTO users (%s) VALUES ('%s');", data[i], info);
                                printf("%s", query);
                                if(mysql_query(con, query)!= 0)
                                {
                                    snprintf(message, sizeof(message), "The data is NOT valid! Try again!");
                                    if(send(cl, message, sizeof(message), 0) < 0) 
                                    {
                                        fprintf(stderr, "[client]Error sending to  client %s\n", data[i]);
                                    }
                                    printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                                    fprintf(stderr, "%s\n", mysql_error(con));
                                }
                                else 
                                {
                                    snprintf(message, sizeof(message), "Your data is valid! ");
                                    printf("%s\n", message);
                                    if(send(cl, message, sizeof(message), 0) < 0) 
                                    {
                                        fprintf(stderr, "[client]Error sending to  client %s\n", data[i]);
                                    }
                                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.IN WHILE\n",idThread);
                                    break;
                                }
                            }
                            else
                            {
                                snprintf(query, sizeof(query), "UPDATE users SET %s = '%s' WHERE user_name like '%s' ;", data[i], info, user_name);
                                printf("%s", query);
                                if(mysql_query(con, query)!=0)
                                {
                                    snprintf(message, sizeof(message), "The data is NOT valid! Try again!");
                                    printf("%s\n", message);
                                    if(send(cl, message, sizeof(message), 0) < 0) 
                                    {
                                        fprintf(stderr, "[client]Error sending to  client %s\n", data[i]);
                                    }
                                    printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                                    fprintf(stderr, "%s\n", mysql_error(con));
                                }
                                else 
                                {
                                    snprintf(message, sizeof(message), "Your data is valid! ");
                                    printf("%s\n", message);
                                    if(send(cl, message, sizeof(message), 0) < 0) 
                                    {
                                        fprintf(stderr, "[client]Error sending to  client %s\n", data[i]);
                                    }
                                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.IN WHILE\n",idThread);
                                    break;
                                }
                            }
                        }
                        else
                        {
                            snprintf(message, sizeof(message), "The data is NOT valid! Try again!");
                            printf("%s\n", message);
                            if(send(cl, message, sizeof(message), 0) < 0) 
                            {
                                fprintf(stderr, "[client]Error sending to  client %s\n", data[i]);
                            }
                            printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                        }
                    }
                    printf("user %s is: %s", data[i], info);
                    snprintf(message, sizeof(message), "%s inregistrat", data[i]);
                    printf("%s\n", message);
                    if(send(cl, message, sizeof(message), 0) < 0) 
                    {
                        fprintf(stderr, "[client]Error sending to  client %s\n", data[i]);
                    }
                }
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                printf("[server] Client already signed in\n");
                nr = 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
            }
        }
        else if(nr == 2)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                fflush(stdout);
                char password[255], isProvider[255];
                printf("[server] client %d wants to sign in\n", idThread);
                loginStatus = 1;
                logoutStatus = 0;
                nr++;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf("[Thread %d] sent succesfully the digit\n", idThread);
                message[0]='\0';
                while(1)
                {
                    user_name[0] = '\0';
                    if (recv(cl, user_name, sizeof(user_name), 0) <= 0) 
                    {
                        fprintf(stderr, "[client]Error receiving from server user_name\n");
                    }
                    printf("%s\n", user_name);
                    user_name[strcspn(user_name, "\n")] = '\0';
                    printf("%ld\n", strlen(user_name));
                    if(strlen(user_name) > 1)
                    {
                        snprintf(query, sizeof(query), "SELECT user_name FROM users WHERE user_name LIKE '%s';", user_name);
                        if (mysql_query(con, query) != 0) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        MYSQL_RES *result;
                        result = mysql_store_result(con);
                        if (result == NULL) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        else 
                        {
                            my_ulonglong num_rows = mysql_num_rows(result);
                            if (num_rows == 0) 
                            {
                                printf("User does not exist.\n");
                                snprintf(message, sizeof(message), "The data is NOT valid! User does not exist. Try again!");
                                if(send(cl, message, sizeof(message), 0) < 0) 
                                {
                                    fprintf(stderr, "[client]Error sending to  client user_name\n");
                                }
                                printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                            } else {
                                MYSQL_ROW row;
                                while ((row = mysql_fetch_row(result)) != NULL) {
                                    printf("User found: %s\n", row[0]); 
                                }
                                snprintf(message, sizeof(message), "Your data is valid");
                                if(send(cl, message, sizeof(message), 0) < 0) 
                                {
                                    fprintf(stderr, "[client]Error sending to  client user_name\n");
                                }
                                printf ("[Thread %d]Mesajul a fost trasmis cu succes.IN WHILE\n",idThread);
                                break;
                            }
                            mysql_free_result(result);
                        }
                    }
                    else
                    {
                        snprintf(message, sizeof(message), "The data is NOT valid! Try again!");
                        if(send(cl, message, sizeof(message), 0) < 0) 
                        {
                            fprintf(stderr, "[client]Error sending to  client user_name\n");
                        }
                        printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                    }
                }
                snprintf(message, sizeof(message), "You entered the user name : %s", user_name);
                if(send(cl, message, sizeof(message), 0) <= 0) 
                {
                        perror("[client]Error recieving message from server \n");
                }
                printf("message sent after user_name\n");
                while(1)
                {
                    password[0] = '\0';
                    if (recv(cl, password, sizeof(password), 0) <= 0) 
                    {
                        fprintf(stderr, "[client]Error receiving from client password\n");
                    }
                    password[strcspn(password, "\n")] = '\0';
                    printf("%s\n", password);
                    printf("%ld\n", strlen(password));
                    if(strlen(password) > 1)
                    {
                        snprintf(query, sizeof(query), "SELECT password FROM users WHERE password LIKE '%s' and user_name LIKE '%s' ;", password, user_name);
                        if (mysql_query(con, query) != 0) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        MYSQL_RES *result;
                         result = mysql_store_result(con);
                        if (result == NULL) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        else 
                        {
                            my_ulonglong num_rows = mysql_num_rows(result);
                            if (num_rows == 0) 
                            {
                                printf("User does not exist.\n");
                                snprintf(message, sizeof(message), "The data is NOT valid! Password is incorrect. Try again!");
                                if(send(cl, message, sizeof(message), 0) < 0) 
                                {
                                    fprintf(stderr, "[client]Error sending to  client user_name\n");
                                }
                                printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                            } else {
                                MYSQL_ROW row;
                                while ((row = mysql_fetch_row(result)) != NULL) {
                                    printf("User found: %s\n", row[0]); 
                                }
                                snprintf(message, sizeof(message), "Your data is valid");
                                if(send(cl, message, sizeof(message), 0) < 0) 
                                {
                                    fprintf(stderr, "[client]Error sending to  client user_name\n");
                                }
                                printf ("[Thread %d]Mesajul a fost trasmis cu succes.IN WHILE\n",idThread);
                                break;
                            }
                            mysql_free_result(result);
                        }
                    }
                    else
                    {
                        snprintf(message, sizeof(message), "The data is NOT valid! Try again!");
                        if(send(cl, message, sizeof(message), 0) < 0) 
                        {
                            fprintf(stderr, "[client]Error sending to  client password\n");
                        }
                        printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                    }                    
                }
                snprintf(message, sizeof(message), "You entered the password : %s", password);
                if(send(cl, message, sizeof(message), 0) <= 0) 
                {
                    perror("[client]Error recieving message from server \n");
                }
                printf("message sent after password\n");
                snprintf(query, sizeof(query), "SELECT isProvider FROM users WHERE user_name LIKE '%s';", user_name);

                if (mysql_query(con, query) != 0) 
                {
                    fprintf(stderr, "Error: %s\n", mysql_error(con));
                }
                MYSQL_RES *result;
                 result = mysql_store_result(con);
                if (result == NULL) {
                    fprintf(stderr, "Error: %s\n", mysql_error(con));
                }
                MYSQL_ROW row;
                if ((row = mysql_fetch_row(result)) != NULL) {
                    isProv = atoi(row[0]);  
                }
                mysql_free_result(result);
                printf("%d", isProv);
                sleep(1);
                if(send(cl, &isProv, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] \n", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes. dupa isProvider\n",idThread);
                printf("[server] Client signed in succesfully\n");
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                printf("[server] Client already signed in\n");
                nr = 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
            }
        }
        else if(nr == 0)
        {
            nr++;
            if (send(cl, &nr, sizeof(int), 0) <= 0) 
            {
                printf("[Thread %d] ", idThread);
                perror("[Thread]Eroare la send() catre client.\n");
            }
            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            break;
        }
        else if(nr == 11)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr = 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf("[Thread %d] is not signed in", idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                loginStatus = 0;
                logoutStatus = 1;
                chsdProv = 0;
                isincart = 0;
                nr++;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf("[Thread %d] signed in", idThread);
                if(send(cl, "See you next time\n", sizeof("See you next time\n"), 0) <= 0) 
                {
                    perror("[client]Error sending message to client \n");
                }
                printf("[Thread %d] sent message to client", idThread);
            } 
            bzero(user_name, sizeof(user_name));
        }
        else if(nr == 3)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr= 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    printf("[Thread %d] client wants to see the suppliers\n", idThread);
                    snprintf(query, sizeof(query), "SELECT id_user, user_name FROM users WHERE isProvider = 1;");
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        char providers[512];
                        providers[0] = '\0';  

                        while ((row = mysql_fetch_row(result)) != NULL) {
                            int id_suplier = atoi(row[0]);  
                            const char *prov_name = row[1];  
                            snprintf(providers + strlen(providers), sizeof(providers) - strlen(providers),
                     "provider id: %d ; provider name: %s\n", id_suplier, prov_name);
                        }
                        mysql_free_result(result);
                        snprintf(message, sizeof(message), "Our providers today are:\n %s", providers);
                    }
                    if(send(cl, message, sizeof(message), 0) <= 0) 
                    {
                        perror("[client]Error sending message to client \n");
                    }
                    printf("[Thread %d] sent message to client", idThread);
                    
                }
                else if(isProv)
                {
                    nr= 0;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                }
            }
        }
        else if(nr == 4)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                 nr= 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    while(1)
                    {
                        snprintf(message, sizeof(message), "Enter the provider id : ");
                        if(send(cl, message, sizeof(message), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                        int option;
                        if(recv(cl, &option, sizeof(int), 0) <= 0)
                        {
                            printf("[Thread %d]\n",idThread);
                            perror ("Eroare la recv() de la client.\n");	
                        }
                        snprintf(query, sizeof(query), "SELECT id_user FROM users WHERE id_user = %d;", option);
                        if (mysql_query(con, query) != 0) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        MYSQL_RES *result;
                        result = mysql_store_result(con);
                        if (result == NULL) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                            snprintf(message, sizeof(message), "The data is NOT valid! User does not exist. Try again!");
                            if(send(cl, message, sizeof(message), 0) < 0) 
                            {
                                fprintf(stderr, "[client]Error sending to  client user_name\n");
                            }
                            printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                        }
                        else 
                        {
                            my_ulonglong num_rows = mysql_num_rows(result);
                            if (num_rows == 0) 
                            {
                                printf("User does not exist.\n");
                                snprintf(message, sizeof(message), "The data is NOT valid! User does not exist. Try again!");
                                if(send(cl, message, sizeof(message), 0) < 0) 
                                {
                                    fprintf(stderr, "[client]Error sending to  client user_name\n");
                                }
                                printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                            } else {
                                MYSQL_ROW row;
                                while ((row = mysql_fetch_row(result)) != NULL) {
                                    printf("User found: %s\n", row[0]); 
                                    id_prov = atoi(row[0]);
                                }
                                printf("%d", id_prov);
                                snprintf(message, sizeof(message), "Provider selected for you");
                                if(send(cl, message, sizeof(message), 0) < 0) 
                                {
                                    fprintf(stderr, "[client]Error sending to  client user_name\n");
                                }
                                printf ("[Thread %d]Mesajul a fost trasmis cu succes.IN WHILE\n",idThread);
                                if(chsdProv == 0)
                                    chsdProv = 1;
                                break;
                            }
                            mysql_free_result(result);
                        }
                    }
                }
                else if(isProv)
                {
                    nr= 0;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                }
            }
        }
        else if(nr == 5)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                 nr= 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    message[0] = '\0';
                    if(chsdProv == 1)
                    {
                        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                        printf("[Thread %d] client wants to see the products\n", idThread);
                        snprintf(query, sizeof(query), "SELECT id_produs, prod_name FROM history WHERE id_prov =%d and status <> 2 and status <> 1;", id_prov);
                        printf("%s", query);
                        if (mysql_query(con, query) != 0) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        MYSQL_RES *result;
                        result = mysql_store_result(con);
                        if (result == NULL) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        else 
                        {
                            MYSQL_ROW row;
                            char product_buffer[512];
                            product_buffer[0] = '\0';  

                            while ((row = mysql_fetch_row(result)) != NULL) {
                                int id_produs = atoi(row[0]);  
                                const char *prod_name = row[1];  
                                snprintf(product_buffer + strlen(product_buffer), sizeof(product_buffer) - strlen(product_buffer),
                        "product id: %d ; product name: %s\n", id_produs, prod_name);
                            }
                            mysql_free_result(result);
                            snprintf(message, sizeof(message), "The products today are :\n %s", product_buffer);
                        }
                        if(strlen(message) < 30)
                        {
                            snprintf(message, sizeof(message), "The market has no products");
                        }
                        printf("%s", message);
                        if(send(cl, message, sizeof(message), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                    else if(chsdProv == 0)
                    {
                        printf("%d - here you can say if is provider", isProv);
                        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                        printf("[Thread %d] client wants to see the products\n", idThread);
                        if(send(cl, "You did not select a provider. Select one to display It's products\n", sizeof("You did not select a provider. Select one to display It's products\n"), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                }
                else if(isProv)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf("%d - here you can say if is provider", isProv);
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    printf("[Thread %d] client wants to see the products\n", idThread);
                    snprintf(query, sizeof(query), "SELECT id_user FROM users WHERE user_name LIKE '%s';", user_name);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                     result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        if ((row = mysql_fetch_row(result)) != NULL) {
                            id_user = atoi(row[0]);  
                        }
                        mysql_free_result(result);
                    }
                    snprintf(query, sizeof(query), "SELECT id_produs, prod_name FROM history WHERE id_prov =%d;", id_user);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        char product_buffer[1024];
                        product_buffer[0] = '\0';  

                        while ((row = mysql_fetch_row(result)) != NULL) {
                            int id_produs = atoi(row[0]);  
                            const char *prod_name = row[1];  
                            snprintf(product_buffer + strlen(product_buffer), sizeof(product_buffer) - strlen(product_buffer),
                     "product id: %d ; product name: %s\n", id_produs, prod_name);
                        }
                        mysql_free_result(result);
                        snprintf(message, sizeof(message), "Your products are:\n %s", product_buffer);
                    }
                    if(strlen(message) < 25)
                    {
                        snprintf(message, sizeof(message), "You have no products!");
                    }
                    printf("%s", message);
                    if(send(cl, message, sizeof(message), 0) <= 0) 
                    {
                        perror("[client]Error sending message to client \n");
                    }
                    printf("[Thread %d] sent message to client", idThread);
                }
            }
        }
        else if(nr == 6)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                 nr= 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                char product_name[255];
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    if(chsdProv == 1)
                    {
                        printf("[Thread %d] client wants to add a product\n", idThread);
                        message[0]='\0';
                        if(flag == 1)
                            snprintf(query, sizeof(query), "SELECT id_produs, prod_name FROM history WHERE status <> 1 and status <> 2;");
                        else
                            snprintf(query, sizeof(query), "SELECT id_produs, prod_name FROM history WHERE id_prov =%d and status <> 1 and status <> 2;", id_prov);
                        if (mysql_query(con, query) != 0) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        MYSQL_RES *result;
                        result = mysql_store_result(con);
                        if (result == NULL) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                        }
                        else 
                        {
                            MYSQL_ROW row;
                            char product_buffer[1024];
                            product_buffer[0] = '\0';  

                            while ((row = mysql_fetch_row(result)) != NULL) {
                                int id_produs = atoi(row[0]);  
                                const char *prod_name = row[1];  
                                snprintf(product_buffer + strlen(product_buffer), sizeof(product_buffer) - strlen(product_buffer),
                        "product id: %d ; product name: %s\n", id_produs, prod_name);
                            }
                            mysql_free_result(result);
                            snprintf(message, sizeof(message), "Your products:\n %s", product_buffer);
                        }
                        printf("%s\n", message);
                        if(strlen(message) < 20)
                        {
                            snprintf(message, sizeof(message), "no available products from this market");
                            if(send(cl, message, sizeof(message), 0) <= 0) 
                            {
                                perror("[client]Error sending message to client \n");
                            }
                            printf("[Thread %d] sent message to client", idThread);
                        }
                        else 
                        {
                        while(1)
                        {
                            snprintf(message, sizeof(message), "Enter the product id:");
                            if(send(cl, message, sizeof(message), 0) <= 0) 
                            {
                                perror("[client]Error sending message to client \n");
                            }
                            printf("[Thread %d] sent message to client", idThread);
                            if(recv(cl, product_name, sizeof(product_name), 0) <= 0)
                            {
                                printf("[Thread %d]\n",idThread);
                                perror ("Eroare la recv() de la client.\n");	
                            }
                            printf("%s", product_name);
                            if(flag == 1)
                                snprintf(query, sizeof(query), "SELECT id_produs FROM history WHERE id_produs = %s and status = 0;", product_name);
                            else
                                snprintf(query, sizeof(query), "SELECT id_produs FROM history WHERE id_produs = %s and status = 0 and id_prov = %d;", product_name, id_prov);
                            if (mysql_query(con, query) != 0) 
                            {
                                fprintf(stderr, "Error: %s\n", mysql_error(con));
                            }
                            MYSQL_RES *result;
                            result = mysql_store_result(con);
                            if (result == NULL) 
                            {
                                fprintf(stderr, "Error: %s\n", mysql_error(con));
                                snprintf(message, sizeof(message), "The product id is NOT valid! Try again!");
                                if(send(cl, message, sizeof(message), 0) < 0) 
                                {
                                    fprintf(stderr, "[client]Error sending to  client user_name\n");
                                }
                                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                            }
                            else 
                            {
                                my_ulonglong num_rows = mysql_num_rows(result);
                                if (num_rows == 0) 
                                {
                                    printf("User does not exist.\n");
                                    snprintf(message, sizeof(message), "The product id is NOT valid! Try again!");
                                    if(send(cl, message, sizeof(message), 0) < 0) 
                                    { 
                                        fprintf(stderr, "[client]Error sending to  client user_name\n");
                                    }
                                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                                } 
                                else 
                                {
                                    snprintf(query, sizeof(query), "SELECT id_user FROM users WHERE user_name LIKE '%s';", user_name);
                                    if (mysql_query(con, query) != 0) 
                                    {
                                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                                    }
                                    result = mysql_store_result(con);
                                    if (result == NULL) 
                                    {
                                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                                    }
                                    else 
                                    {
                                        MYSQL_ROW row;
                                        if ((row = mysql_fetch_row(result)) != NULL) {
                                            id_user = atoi(row[0]);  
                                        }
                                        mysql_free_result(result);
                                    }
                                    if(flag == 1)
                                        snprintf(query, sizeof(query), "UPDATE history SET status = 1, id_ben = %d where id_produs = %s and status = 0;", id_user,product_name);
                                    else
                                        snprintf(query, sizeof(query), "UPDATE history SET status = 1, id_ben = %d where id_produs = %s and status = 0 and id_prov = %d;", id_user,product_name, id_prov);
                                    printf("%s", query);
                                    if(mysql_query(con, query))
                                    {
                                        fprintf(stderr, "%s\n", mysql_error(con));
                                        snprintf(message, sizeof(message), "The product id is NOT valid! Try again!");
                                        if(send(cl, message, sizeof(message), 0) < 0) 
                                        {
                                            fprintf(stderr, "[client]Error sending to  client user_name\n");
                                        }
                                        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread); 
                                    }
                                    else 
                                    {
                                        snprintf(message, sizeof(message), "We recorded your order");
                                        if(send(cl, message, sizeof(message), 0) <= 0) 
                                        {
                                            perror("[client]Error sending message to client \n");
                                        }
                                        printf("[Thread %d] sent message to client\n", idThread);
                                        break;
                                    }
                                }
                            }
                        }
                        }
                    }
                    else if(chsdProv == 0)
                    {
                        printf("%d - here you can say if is provider", isProv);
                        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                        printf("[Thread %d] client wants to see the products\n", idThread);
                        if(send(cl, "You did not select a provider. Select one to display It's products\n", sizeof("You did not select a provider. Select one to display It's products\n"), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                }
                else if(isProv==1)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    printf("[Thread %d] client wants to add a product\n", idThread);
                    snprintf(message, sizeof(message), "Enter the product");
                    if(send(cl, message, sizeof(message), 0) <= 0) 
                    {
                        perror("[client]Error sending message to client \n");
                    }
                    printf("[Thread %d] sent message to client", idThread);
                    snprintf(query, sizeof(query), "SELECT id_user FROM users WHERE user_name LIKE '%s';", user_name);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        if ((row = mysql_fetch_row(result)) != NULL) {
                            id_user = atoi(row[0]);  
                        }
                        mysql_free_result(result);
                    }
                    bzero(product_name, sizeof(product_name));
                    if(recv(cl, product_name, sizeof(product_name), 0) <= 0)
                    {
                        printf("[Thread %d]\n",idThread);
                        perror ("Eroare la recv() de la client.\n");	
                    }
                    printf("%s", product_name);
                    product_name[strcspn(product_name, "\n")] = '\0';
                    snprintf(query, sizeof(query), "INSERT INTO history (prod_name, id_prov) VALUES ('%s', '%d');", product_name, id_user);
                    printf("%s", query);
                    if(mysql_query(con, query))
                    {
                        snprintf(message, sizeof(message), "A problem occurred! Try again!");  
                        printf("%s", message); 
                        if(send(cl, message, sizeof(message), 0) < 0) 
                        {
                            fprintf(stderr, "[client]Error sending to  client SQL ERROR \n");
                        }
                        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                        fprintf(stderr, "%s\n", mysql_error(con));
                    }
                    else 
                    {
                        bzero(message, sizeof(message));
                        snprintf(message, sizeof(message), "We added the product.");
                        printf("%s\n", message);
                        if(send(cl, message, sizeof(message), 0) < 0) 
                        {
                            fprintf(stderr, "[client]Error sending to  client message of adding product\n");
                        }
                        printf ("[Thread %d]Mesajul a fost trasmis cu succes. dupa adaugare produs\n",idThread);
                    }
                }
            }
        }
        else if(nr == 8)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr= 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    printf("[Thread %d] client wants to see history of donations\n", idThread);
                    snprintf(query, sizeof(query), "SELECT id_user FROM users WHERE user_name LIKE '%s';", user_name);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                     result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        if ((row = mysql_fetch_row(result)) != NULL) {
                            id_user = atoi(row[0]);  
                        }
                        mysql_free_result(result);
                    }
                    snprintf(query, sizeof(query), "SELECT id_produs, prod_name FROM history WHERE id_ben = %d and status = 2;", id_user);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        char product_buffer[512];
                        product_buffer[0] = '\0';  

                        while ((row = mysql_fetch_row(result)) != NULL) {
                            int id_produs = atoi(row[0]);  
                            const char *prod_name = row[1];  
                            snprintf(product_buffer + strlen(product_buffer), sizeof(product_buffer) - strlen(product_buffer),
                    "product id: %d ; product name: %s\n", id_produs, prod_name);
                        }
                        mysql_free_result(result);
                        snprintf(message, sizeof(message), "Your History of orders is:\n %s", product_buffer);
                    }
                    if(send(cl, message, sizeof(message), 0) <= 0) 
                    {
                        perror("[client]Error sending message to client \n");
                    }
                    printf("[Thread %d] sent message to client", idThread);
                }
                else if(isProv)
                {
                    nr= 0;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                }
            }
        }
        else if(nr == 7)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr= 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    snprintf(query, sizeof(query), "SELECT id_produs FROM history WHERE id_ben = %d and status = 1;", id_user);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                        snprintf(message, sizeof(message), "A problem occured");
                        if(send(cl, message, sizeof(message), 0) < 0) 
                        {
                            fprintf(stderr, "[client]Error sending to  client user_name\n");
                        }
                        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    }
                    else 
                    {
                        my_ulonglong num_rows = mysql_num_rows(result);
                        if (num_rows == 0) 
                        {
                             printf("[Thread %d] client wants to place a order\n", idThread);
                            snprintf(message, sizeof(message), "Add a product in your list first");
                            if(send(cl, message, sizeof(message), 0) <= 0) 
                            {
                                perror("[client]Error sending message to client \n");
                            }
                            printf("[Thread %d] sent message to client", idThread);
                        }
                        else
                        {
                            snprintf(query, sizeof(query), "UPDATE history SET status = 2 where id_ben = %d and status = 1;", id_user);
                            printf("%s", query);
                            if(mysql_query(con, query))
                            {
                                fprintf(stderr, "%s\n", mysql_error(con));
                                snprintf(message, sizeof(message), "A problem ocurred. Try again!");
                                if(send(cl, message, sizeof(message), 0) <= 0) 
                                {
                                    perror("[client]Error sending message to client \n");
                                }
                                printf("[Thread %d] sent message to client", idThread);
                            }
                            else
                            {
                                printf("[Thread %d] client wants to place a order\n", idThread);
                                snprintf(message, sizeof(message), "We will deliver the order in the shortest time");
                                if(send(cl, message, sizeof(message), 0) <= 0) 
                                {
                                    perror("[client]Error sending message to client \n");
                                }
                                printf("[Thread %d] sent message to client", idThread);
                            }
                        }
                    }
                }
                else if(isProv)
                {
                    nr= 0;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                }
            }
        }
        else if(nr == 9)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr= 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    printf("[Thread %d] client wants to see history of donations\n", idThread);
                    snprintf(query, sizeof(query), "SELECT id_user FROM users WHERE user_name LIKE '%s';", user_name);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                     result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        if ((row = mysql_fetch_row(result)) != NULL) {
                            id_user = atoi(row[0]);  
                        }
                        mysql_free_result(result);
                    }
                    snprintf(query, sizeof(query), "SELECT id_produs, prod_name FROM history WHERE id_ben = %d and status = 1;", id_user);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        isincart = 1;
                        char product_buffer[512];
                        product_buffer[0] = '\0';  

                        while ((row = mysql_fetch_row(result)) != NULL) {
                            int id_produs = atoi(row[0]);  
                            const char *prod_name = row[1];  
                            snprintf(product_buffer + strlen(product_buffer), sizeof(product_buffer) - strlen(product_buffer),
                    "product id: %d ; product name: %s\n", id_produs, prod_name);
                        }
                        mysql_free_result(result);
                        snprintf(message, sizeof(message), "Your product cart is:\n %s", product_buffer);
                    }
                    if(send(cl, message, sizeof(message), 0) <= 0) 
                    {
                        perror("[client]Error sending message to client \n");
                    }
                    printf("[Thread %d] sent message to client", idThread);
                    if(strlen(message) < 30)
                        iszero = 0;
                    else
                        iszero = 1;

                }
                else if(isProv)
                {
                    nr= 0;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                }
            }
        }
        else if(nr == 10)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr= 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                char product_name[255];
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    if(isincart)
                    {
                        if(iszero == 0)
                        {
                            snprintf(message, sizeof(message), "you have no product to delete");
                            if(send(cl, message, sizeof(message), 0) <= 0) 
                            {
                                perror("[client]Error sending message to client \n");
                            }
                            printf("[Thread %d] sent message to client", idThread);
                        }
                        else
                        {
                        while(1)
                        {
                            snprintf(message, sizeof(message), "Enter the product id you want to :");
                            if(send(cl, message, sizeof(message), 0) <= 0) 
                            {
                                perror("[client]Error sending message to client \n");
                            }
                            printf("[Thread %d] sent message to client", idThread);
                            if(recv(cl, product_name, sizeof(product_name), 0) <= 0)
                            {
                                printf("[Thread %d]\n",idThread);
                                perror ("Eroare la recv() de la client.\n");	
                            }
                            printf("%s", product_name);
                            product_name[strcspn(product_name, "\n")] = '\0';
                            snprintf(query, sizeof(query), "SELECT id_produs FROM history WHERE id_produs = %s and status = 1;", product_name);
                            if (mysql_query(con, query) != 0) 
                            {
                                fprintf(stderr, "Error: %s\n", mysql_error(con));
                            }
                            MYSQL_RES *result;
                            result = mysql_store_result(con);
                            if (result == NULL) 
                            {
                                fprintf(stderr, "Error: %s\n", mysql_error(con));
                                snprintf(message, sizeof(message), "The product id is NOT valid! Try again!");
                                if(send(cl, message, sizeof(message), 0) < 0) 
                                {
                                    fprintf(stderr, "[client]Error sending to  client user_name\n");
                                }
                                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                            }
                            else 
                            {
                                my_ulonglong num_rows = mysql_num_rows(result);
                                if (num_rows == 0) 
                                {
                                    snprintf(message, sizeof(message), "The product id is NOT valid! Try again!");
                                    if(send(cl, message, sizeof(message), 0) < 0) 
                                    { 
                                        fprintf(stderr, "[client]Error sending to  client user_name\n");
                                    }
                                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                                } 
                                else 
                                {
                                    snprintf(query, sizeof(query), "UPDATE history SET status = 0, id_ben = NULL where id_produs = %s and status = 1;",product_name);
                                    if(mysql_query(con, query))
                                    {
                                        fprintf(stderr, "%s\n", mysql_error(con));
                                        snprintf(message, sizeof(message), "The product id is NOT valid! Try again!");
                                        if(send(cl, message, sizeof(message), 0) < 0) 
                                        {
                                            fprintf(stderr, "[client]Error sending to  client user_name\n");
                                        }
                                        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread); 
                                    }
                                    else 
                                    {
                                        snprintf(message, sizeof(message), "We deleted the product from your product cart");
                                        if(send(cl, message, sizeof(message), 0) <= 0) 
                                        {
                                            perror("[client]Error sending message to client \n");
                                        }
                                        printf("[Thread %d] sent message to client\n", idThread);
                                        break;
                                    }
                                }
                            }   
                        }
                        }
                    }
                    else if(isincart == 0)
                    {
                        printf("[Thread %d] client wants to see the products\n", idThread);
                        snprintf(message, sizeof(message), "You did not enter your product cart. Enter it to proceed with it");
                        if(send(cl, message, sizeof(message), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                }
                else if(isProv)
                {
                    MYSQL_ROW row;
                     nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    int status = 0;
                    snprintf(query, sizeof(query), "SELECT id_user FROM users WHERE user_name LIKE '%s';", user_name);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error extragere user: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                     result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error extragere info din interogare user: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        
                        if ((row = mysql_fetch_row(result)) != NULL) {
                            id_user = atoi(row[0]);  
                        }
                        mysql_free_result(result);
                    }
                    snprintf(query, sizeof(query), "SELECT id_produs, prod_name FROM history WHERE id_prov =%d;", id_user);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error selectare produse user : %s\n", mysql_error(con));
                    }
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        char product_buffer[1024];
                        product_buffer[0] = '\0';  

                        while ((row = mysql_fetch_row(result)) != NULL) {
                            int id_produs = atoi(row[0]);  
                            const char *prod_name = row[1];  
                            snprintf(product_buffer + strlen(product_buffer), sizeof(product_buffer) - strlen(product_buffer),
                     "product id: %d ; product name: %s\n", id_produs, prod_name);
                        }
                        mysql_free_result(result);
                        snprintf(message, sizeof(message), "Your products are:\n %s", product_buffer);
                    }
                    if(send(cl, message, sizeof(message), 0) <= 0) 
                    {
                        perror("[client]Error sending message to client \n");
                    }
                    printf("[Thread %d] sent message to client", idThread);
                    if(strlen(message) < 30)
                        iszero = 0;
                    else
                        iszero = 1;
                        if(iszero == 0)
                        {
                            snprintf(message, sizeof(message), "you have no product to delete");
                            if(send(cl, message, sizeof(message), 0) <= 0) 
                            {
                                perror("[client]Error sending message to client \n");
                            }
                            printf("[Thread %d] sent message to client", idThread);
                        }
                        else
                        {
                            while(1)
                            {
                                snprintf(message, sizeof(message), "Enter the product id:");
                                if(send(cl, message, sizeof(message), 0) <= 0) 
                                {
                                    perror("[client]Error sending message to client \n");
                                }
                                printf("[Thread %d] sent message to client", idThread);
                                if(recv(cl, product_name, sizeof(product_name), 0) <= 0)
                                {
                                    printf("[Thread %d]\n",idThread);
                                    perror ("Eroare la recv() de la client.\n");	
                                }
                                product_name[strcspn(product_name, "\n")] = '\0';
                                printf("here is id_prod %sheeereeee", product_name);
                                snprintf(query, sizeof(query), "SELECT id_produs FROM history WHERE id_produs = %s and id_prov = %d;", product_name, id_user);
                                printf("%s\n", query);
                                if (mysql_query(con, query) != 0) 
                                {
                                    fprintf(stderr, "Error la validare id produs : %s\n", mysql_error(con));
                                }
                                MYSQL_RES *result;
                                result = mysql_store_result(con);
                                if (result == NULL) 
                                {
                                    fprintf(stderr, "Error la result la validare id produs: %s\n", mysql_error(con));
                                    snprintf(message, sizeof(message), "The product id is NOT valid! Try again!");
                                    if(send(cl, message, sizeof(message), 0) < 0) 
                                    {
                                        fprintf(stderr, "[client]Error sending to  client user_name\n");
                                    }
                                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                                }
                                else
                                {
                                    my_ulonglong num_rows = mysql_num_rows(result);
                                    if (num_rows == 0) 
                                    {
                                        snprintf(message, sizeof(message), "The product id is NOT valid! Try again!");
                                        if(send(cl, message, sizeof(message), 0) < 0) 
                                        { 
                                            fprintf(stderr, "[client]Error sending to  client user_name\n");
                                        }
                                        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                                    } 
                                    else 
                                    {
                                        snprintf(query, sizeof(query), "SELECT status from history where id_produs = %s;", product_name);
                                        printf("%s\n", query);
                                        if (mysql_query(con, query) != 0) 
                                        {
                                            fprintf(stderr, "Error la selectare status : %s\n", mysql_error(con));
                                            snprintf(message, sizeof(message), "An error ocurred. Try again!");
                                            if(send(cl, message, sizeof(message), 0) <= 0) 
                                            {
                                                perror("[client]Error sending message to client \n");
                                            }
                                            printf("[Thread %d] sent message to client", idThread);
                                        } 
                                        else 
                                        {
                                            //mysql_free_result(result);
                                            result = mysql_store_result(con);
                                            if (result != NULL) 
                                            {
                                                MYSQL_ROW row;
                                                if ((row = mysql_fetch_row(result)) != NULL) 
                                                {
                                                    status = atoi(row[0]);
                                                    printf("Status: %d\n", status);
                                                    if(status == 0 || status == 1)
                                                    {
                                                        snprintf(query, sizeof(query), "DELETE FROM history where id_produs = %s and id_prov = %d;", product_name, id_user);
                                                        printf("%s", query);
                                                        if(mysql_query(con, query)!=0)
                                                        {
                                                            fprintf(stderr, "Error la delete : %s\n", mysql_error(con));
                                                            snprintf(message, sizeof(message), "An error ocurred. Try again!");
                                                            if(send(cl, message, sizeof(message), 0) <= 0) 
                                                            {
                                                                perror("[client]Error sending message to client \n");
                                                            }
                                                            printf("[Thread %d] sent message to client", idThread);
                                                        }
                                                        else
                                                        {
                                                            snprintf(message, sizeof(message), "We deleted the product !");
                                                            if(send(cl, message, sizeof(message), 0) <= 0) 
                                                            {
                                                                perror("[client]Error sending message to client \n");
                                                            }
                                                            printf("[Thread %d] sent message to client inainte de break", idThread);
                                                            break;
                                                        }
                                                    }
                                                    else if(status == 2)
                                                    {
                                                        snprintf(query, sizeof(query), "UPDATE history SET id_prov = NULL where id_produs = %s and id_prov = %d;", product_name, id_user);
                                                        printf("%s\n", query);
                                                        if(mysql_query(con, query)!=0)
                                                        {
                                                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                                                            snprintf(message, sizeof(message), "An error ocurred. Try again!");
                                                            if(send(cl, message, sizeof(message), 0) <= 0) 
                                                            {
                                                                perror("[client]Error sending message to client \n");
                                                            }
                                                            printf("[Thread %d] sent message to client  inainte de break", idThread);
                                                        }
                                                        else
                                                        {
                                                            snprintf(message, sizeof(message), "We deleted the product !");
                                                            if(send(cl, message, sizeof(message), 0) <= 0) 
                                                            {
                                                                perror("[client]Error sending message to client \n");
                                                            }
                                                            printf("[Thread %d] sent message to client  inainte de break", idThread);
                                                            break;
                                                        }
                                                    }
                                                } 
                                                
                                            } 
                                            else 
                                            {
                                                snprintf(message, sizeof(message), "An error ocurred. Try again!");
                                                if(send(cl, message, sizeof(message), 0) <= 0) 
                                                {
                                                    perror("[client]Error sending message to client \n");
                                                }
                                                printf("[Thread %d] sent message to client", idThread);
                                                fprintf(stderr, "Error: %s\n", mysql_error(con));
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    printf("\nhere inafara de while\n");
                }
            }
        }
        else if(nr == 12)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr = 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf("[Thread %d] is not signed in", idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    char product_name[255];
                    product_name[0] = '\0';
                    if(recv(cl, product_name, sizeof(product_name), 0) <= 0)
                    {
                        printf("[Thread %d]\n",idThread);
                        perror ("Eroare la recv() de la client.\n");	
                    }
                    printf("%s aici e string de la client\n", product_name);
                    product_name[strcspn(product_name, "\n")] = '\0';
                    snprintf(query, sizeof(query), "SELECT id_produs , prod_name FROM history WHERE UPPER(REPLACE(prod_name, ' ', '')) like CONCAT('%', UPPER(REPLACE('%s', ' ' , '')), '%') and status <> 2 and status <> 1;", product_name);
                    printf("%s", query);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                        snprintf(message, sizeof(message), "An error ocurred. Try again later!");
                        if(send(cl, message, sizeof(message), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        char product_buffer[512];
                        product_buffer[0] = '\0';
                        while ((row = mysql_fetch_row(result)) != NULL) 
                        {
                            int id_produs = atoi(row[0]);  
                            const char *prod_name = row[1];  
                            snprintf(product_buffer + strlen(product_buffer), sizeof(product_buffer) - strlen(product_buffer),
                    "product id: %d ; product name: %s\n", id_produs, prod_name);
                        }
                        mysql_free_result(result);
                        snprintf(message, sizeof(message), "These are the products we found:\n %s", product_buffer);
                        printf("%s", product_buffer);
                        if(strlen(message) < 40)
                        {
                            snprintf(message, sizeof(message), "We found no products");
                        }
                        else 
                        {
                            chsdProv = 1;
                            flag = 1;
                        }
                        printf("%s\n", message);
                        if(send(cl, message, sizeof(message), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                }
                else if(isProv)
                {
                    nr= 0;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                }

            }
        }
        else if(nr == 13)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr = 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf("[Thread %d] is not signed in", idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                if(isProv == 0)
                {
                    nr++;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                    char provider_name[255];
                    provider_name[0] = '\0';
                    if(recv(cl, provider_name, sizeof(provider_name), 0) <= 0)
                    {
                        printf("[Thread %d]\n",idThread);
                        perror ("Eroare la recv() de la client.\n");	
                    }
                    printf("%s aici e string de la client\n", provider_name);
                    provider_name[strcspn(provider_name, "\n")] = '\0';
                    snprintf(query, sizeof(query), "SELECT id_user , user_name FROM users WHERE UPPER(REPLACE(user_name, ' ', '')) like CONCAT('%', UPPER(REPLACE('%s', ' ' , '')), '%');", provider_name);
                    printf("%s", query);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                        snprintf(message, sizeof(message), "An error ocurred. Try again later!");
                        if(send(cl, message, sizeof(message), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        char product_buffer[512];
                        product_buffer[0] = '\0';
                        while ((row = mysql_fetch_row(result)) != NULL) 
                        {
                            int id_produs = atoi(row[0]);  
                            const char *prod_name = row[1];  
                            snprintf(product_buffer + strlen(product_buffer), sizeof(product_buffer) - strlen(product_buffer),
                    "market id: %d ; market name: %s\n", id_produs, prod_name);
                        }
                        mysql_free_result(result);
                        snprintf(message, sizeof(message), "These are the markets ant restaurants we found:\n %s", product_buffer);
                        printf("%s", product_buffer);
                        if(strlen(message) < 40)
                        {
                            snprintf(message, sizeof(message), "We found no markets or restaurants with this name. Try something else!");
                        }
                        printf("%s\n", message);
                        if(send(cl, message, sizeof(message), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                }
                else if(isProv)
                {
                    nr= 0;
                    if (send(cl, &nr, sizeof(int), 0) <= 0) 
                    {
                        printf("[Thread %d] ", idThread);
                        perror("[Thread]Eroare la send() catre client.\n");
                    }
                    printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                }

            }
        }
        else if(nr == 14)
        {
            if(loginStatus == 0 && logoutStatus == 1)
            {
                nr = 0;
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf("[Thread %d] is not signed in", idThread);
            }
            else if(loginStatus == 1 && logoutStatus == 0)
            {
                nr++;
                char answer[255];
                if (send(cl, &nr, sizeof(int), 0) <= 0) 
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                snprintf(message, sizeof(message), "are you sure you want to delete your account permanently? [yes/no]");
                if(send(cl, message, sizeof(message), 0) <= 0)
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la send() catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
                if(recv(cl, answer, sizeof(answer), 0) <= 0)
                {
                    printf("[Thread %d] ", idThread);
                    perror("[Thread]Eroare la recv() answer catre client.\n");
                }
                printf ("[Thread %d]Mesajul a fost primit cu succes.\n",idThread);
                printf("%ld\n", strlen(answer));
                answer[strcspn(answer, "\n")] = '\0';
                if(strstr(answer, "yes") && strlen(answer) == 3)
                {
                    int succes = 0;
                    snprintf(query, sizeof(query), "SELECT id_user FROM users WHERE user_name = '%s';", user_name);
                    if (mysql_query(con, query) != 0) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                    }
                    MYSQL_RES *result;
                    result = mysql_store_result(con);
                    if (result == NULL) 
                    {
                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                        snprintf(message, sizeof(message), "An error ocurred. Try again later!");
                        if(send(cl, message, sizeof(message), 0) <= 0) 
                        {
                            perror("[client]Error sending message to client \n");
                        }
                        printf("[Thread %d] sent message to client", idThread);
                    }
                    else 
                    {
                        MYSQL_ROW row;
                        if ((row = mysql_fetch_row(result)) != NULL) {
                            id_user = atoi(row[0]);  
                        }
                        printf("%d\n", id_user);
                        mysql_free_result(result);
                        if(isProv)
                            snprintf(query, sizeof(query), "UPDATE history SET id_prov = NULL where id_prov = %d;", id_user);
                        else if(isProv == 0)
                            snprintf(query, sizeof(query), "UPDATE history SET id_ben = NULL where id_ben = %d;", id_user);
                        printf("%s\n", query);
                        if (mysql_query(con, query) != 0) 
                        {
                            fprintf(stderr, "Error: %s\n", mysql_error(con));
                            snprintf(message, sizeof(message), "An error ocurred. Try again later!");
                            if(send(cl, message, sizeof(message), 0) <= 0) 
                            {
                                perror("[client]Error sending message to client \n");
                            }
                            printf("[Thread %d] sent message to client", idThread);
                        }
                        else
                        {
                            while(1)
                            {
                                answer[0] = '\0';
                                if (recv(cl, answer, sizeof(answer), 0) <= 0) 
                                {
                                    fprintf(stderr, "[client]Error receiving from client password\n");
                                }
                                answer[strcspn(answer, "\n")] = '\0';
                                printf("%s\n", answer);
                                printf("%ld\n", strlen(answer));
                                if(strlen(answer) > 1)
                                {
                                    snprintf(query, sizeof(query), "SELECT password FROM users WHERE password LIKE '%s' and user_name LIKE '%s' ;", answer, user_name);
                                    if (mysql_query(con, query) != 0) 
                                    {
                                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                                    }
                                    MYSQL_RES *result;
                                    result = mysql_store_result(con);
                                    if (result == NULL) 
                                    {
                                        fprintf(stderr, "Error: %s\n", mysql_error(con));
                                    }
                                    else 
                                    {
                                        my_ulonglong num_rows = mysql_num_rows(result);
                                        if (num_rows == 0) 
                                        {
                                            printf("User does not exist.\n");
                                            snprintf(message, sizeof(message), "The data is NOT valid! Password is incorrect. Try again!");
                                            if(send(cl, message, sizeof(message), 0) < 0) 
                                            {
                                                fprintf(stderr, "[client]Error sending to  client user_name\n");
                                            }
                                            printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                                        } else {
                                            MYSQL_ROW row;
                                            while ((row = mysql_fetch_row(result)) != NULL) {
                                                printf("User found: %s\n", row[0]); 
                                            }
                                            snprintf(message, sizeof(message), "Your data is valid");
                                            if(send(cl, message, sizeof(message), 0) < 0) 
                                            {
                                                fprintf(stderr, "[client]Error sending to  client user_name\n");
                                            }
                                            printf ("[Thread %d]Mesajul a fost trasmis cu succes.IN WHILE\n",idThread);
                                            break;
                                        }
                                        mysql_free_result(result);
                                    }
                                }
                                else
                                {
                                    snprintf(message, sizeof(message), "The data is NOT valid! Try again!");
                                    if(send(cl, message, sizeof(message), 0) < 0) 
                                    {
                                        fprintf(stderr, "[client]Error sending to  client password\n");
                                    }
                                    printf ("[Thread %d]Mesajul a fost trasmis cu succes. in WHILE\n",idThread);
                                }                    
                            }
                            snprintf(query, sizeof(query), "DELETE FROM users WHERE id_user = %d;" , id_user);
                            if (mysql_query(con, query) != 0) 
                            {
                                fprintf(stderr, "Error: %s\n", mysql_error(con));
                                snprintf(message, sizeof(message), "An error ocurred. Try again later!");
                                if(send(cl, message, sizeof(message), 0) <= 0) 
                                {
                                    perror("[client]Error sending message to client \n");
                                }
                                printf("[Thread %d] sent message to client", idThread);
                            }
                            else
                            {
                                loginStatus = 0;
                                logoutStatus = 1;
                                chsdProv = 0;
                                isincart = 0;
                                snprintf(message, sizeof(message), "We're sorry to lose you! Have a nice day!");
                                if(send(cl, message, sizeof(message), 0) <= 0) 
                                {
                                    perror("[client]Error sending message to client \n");
                                }
                                printf("[Thread %d] sent message to client", idThread);
                            }
                        }
                    }
                }
                else 
                {
                    snprintf(message, sizeof(message), "Thank you for staying!");
                    if(send(cl, message, sizeof(message), 0) <= 0) 
                    {
                        perror("[client]Error sending message to client \n");
                    }
                    printf("[Thread %d] sent message to client", idThread);
                }
            }
        }
        else 
        {
            nr= 0;
            if (send(cl, &nr, sizeof(int), 0) <= 0) 
            {
                printf("[Thread %d] ", idThread);
                perror("[Thread]Eroare la send() catre client.\n");
            }
            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);
        }
    }
}

static void *treat(void *arg)
{
    int client;
    struct sockaddr_in from;
    printf("[thread] : %d - pornit...\n", (int) arg);
    fflush(stdout);

    for( ; ; )
    {
        int length = sizeof(from);
        pthread_mutex_lock(&mlock);
        if((client = accept(socket_desc, (struct sockaddr *)&from, &length)) < 0)
            perror("[thread]Eroare la accept().\n");
        pthread_mutex_unlock(&mlock);
        threadPool[(int)arg].thCount++;

        raspunde(client, (int)arg);
        close(client);
    }
}

void createThread(int i)
{
    void *treat(void *);
    pthread_create(&threadPool[i].idThread, NULL, &treat, (void*)i);
    return;
} 

void handleErrorMysql(MYSQL *con)
{
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}

int main(int argc, char *argv[])
{
    void createThread(int);
    struct sockaddr_in server;

    if(argc < 2)
    {
        fprintf(stderr,"Eroare: Introduceti numarul de fire de executie...");
        exit(1);
    }

    nrThreads = atoi(argv[1]);
    if(nrThreads <= 0)
    {
        fprintf(stderr,"Eroare: Numar de fire invalid...");
        exit(1);
    }
    con = mysql_init(NULL);
    if (con == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, "localhost", "temarc2", "password", NULL, 0, NULL, 0) == NULL)
        handleErrorMysql(con);

    if (mysql_query(con, "use food_waste_reducer"))
        handleErrorMysql(con);

    threadPool = calloc(sizeof(Thread), nrThreads);
    if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }

    int on = 1;
    setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind (socket_desc, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server]Eroare la bind().\n");
        return errno;
    }

    if(listen(socket_desc, 2) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }
    printf("Nr threaduri %d \n", nrThreads); fflush(stdout);

    for(int i = 0; i < nrThreads; i++)
        createThread(i);

    for( ; ;)
    {
        printf ("[server]Asteptam la portul %d...\n",PORT);
        pause();
    }
    mysql_close(con);
    exit(0);
}
