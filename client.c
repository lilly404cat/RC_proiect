#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

extern int errno;

int port;
int socked_desc;
int nr;
void showMainMenu_welcome()
{
    printf("Welcome to Our charity!\nHow can we help you?\n");
    printf("To sign up type: 1\nTo sign in type 2\nTo exit the app type 0\n");    
}

void showMainMenu_actions_client()
{
    printf("To display the markets and restaurants type 3\nTo select a market or a restaurant type 4\nTo display all available products type 5\nTo select a product type 6\nTo place your order type 7\nTo display history type 8\nTo display product cart type 9\nTo delete a product from your product cart type 10\nTo search a product type 12\nTo search a market/restaurant type 13\nTo sign out type 11\nTo exit the app type 0\nTo delete your account type 14\n");
}
void showMainMenu_actions_provider()
{
    printf("To display your products type 5\nTo add a product type 6\nTo delete a product type 10\nTo sign out type 11\nTo exit the app type 0\nTo delete your account type 14\n");
}

void chooseTheTypeofMenu(int option)
{
    if(option == 1)
        showMainMenu_welcome();
    else if(option == 2)
        showMainMenu_actions_client();
    else if(option == 3)
        showMainMenu_actions_provider();
}

int signup()
{
    char *data[6] ={"username", "password", "email", "phone", "location", "isProvider"};
    char info[255], message[2048];
    int isProv;
    for( int i = 0; i < 6; i++)
    {
        printf("\n");
        printf("Enter %s\n", data[i]);
        
        while(1)
        {
            read(0, info, sizeof(info));
            if(send(socked_desc, info, sizeof(info), 0) <= 0) 
            {
                fprintf(stderr, "[client]Error sending to server %s\n", data[i]);
            }
            if(i == 5)
            {
                if (recv(socked_desc, &isProv, sizeof(int), 0) <= 0) 
                {
                    perror("[Cleint]Eroare la receive() from server.\n");
                }
                printf ("[Client]Mesajul a fost primit cu succes.\n");
                break;
            }
            if (recv(socked_desc, message, sizeof(message), 0) <= 0) 
            {
                perror("[Cleint]Eroare la receive() from server.\n");
            }
            printf("%s\n", message);
            if(strstr(message, "Your")) {
                break;
            }
        }
        if(recv(socked_desc, message, sizeof(message), 0) < 0) 
        {
            perror("[client]Error recieving message from server \n");
        }
        printf("%s\n", message);
    }
    //printf("\n%d\n", isProv);
    return isProv;
}

int signin()
{
    int isProv;
    char si_method[255], password[255], message_server[2048], isprovider[255];

    memset(message_server, 0, sizeof(message_server));
    printf("Enter username: \n");
    while(1)
    {
        memset(si_method, 0, sizeof(si_method));
        read(0, si_method, sizeof(si_method));
        //printf("%s\n", si_method);
        if(send(socked_desc, si_method, sizeof(si_method), 0) <= 0) 
        {
            fprintf(stderr, "[client]Error sending to server username \n");
        }
        if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
        {
            perror("[client]Error recieving message from server \n");
        }
        printf("%s\n", message_server);
        if(strstr(message_server, "Your")) 
            break;
    }
    if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
    {
        perror("[client]Error recieving message from server \n");
    }
    printf("%s Great!\n", message_server);
    printf("Enter password : \n");
    memset(message_server, 0, sizeof(message_server));
    memset(password, 0, sizeof(password));
    while(1)
    {
        read(0, password, sizeof(password));
        //printf("%s\n", password);
        if(send(socked_desc, password, sizeof(password), 0) <= 0) 
        {
            fprintf(stderr, "[client]Error sending to server \n");
        }
        if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
        {
            perror("[client]Error recieving message from server \n");
        }
        printf("%s\n", message_server);
        if(strstr(message_server, "Your")) 
            break;
    }
    if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
    {
        perror("[client]Error recieving message from server \n");
    }
    // sleep(1);
    printf("%s\n", message_server);
    memset(message_server, 0, sizeof(message_server));
    if(recv(socked_desc, &isProv, sizeof(int), 0) <= 0) 
    {
        perror("[Cleint]Eroare la receive() from server.\n");
    }
    //printf("%d", isProv);
    printf ("[Client]Mesajul a fost primit cu succes.\n");
    return isProv;
}

int main (int argc, char *argv[])
{
    int exit_code = 0, menuoption = 1;			
    struct sockaddr_in server;
    char buf[10], info[256];

    if(argc != 3)
    {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[2]);

    if((socked_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (port);
  
    if(connect(socked_desc, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }
    int isProv ;
    while(!exit_code)
    {
        char message_server[2048];
        fflush(stdout);
        nr=0;
        bzero(buf, 10);
        chooseTheTypeofMenu(menuoption); 
        read(0, buf, sizeof(buf));
        nr=atoi(buf);
        //printf("[client] Am citit %d\n",nr);
        if(send(socked_desc, &nr, sizeof(int), 0) <= 0) 
        {
            perror("[client]Eroare la send() spre server.\n");
            return errno;
        }
        
        if(recv(socked_desc, &nr, sizeof(int), 0) < 0) 
        {
            perror("[client]Eroare la recv() de la server.\n");
            return errno;
        }
        printf ("[client]Mesajul primit este: %d\n", nr);

        if(nr == 2)
        {
            isProv = signup();
            //printf("\n%d\n", isProv);
            if(isProv == 0)
                menuoption = 2;
            else 
                menuoption = 3;
            printf("\n");
        }
        else if(nr == 3)
        {
            isProv = signin();
            //printf("\n%d\n", isProv);
            if(isProv == 0)
                menuoption = 2;
            else 
                menuoption = 3;
            printf("\n");
        }
        else if(nr == 4)
        {
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                fprintf(stderr, "[client]Error recieving message from server la comanda 4\n");
            }
            printf("%s\n", message_server);
        }
        else if(nr == 5 )
        {
            while(1)
            {
                if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                {
                    fprintf(stderr, "[client]Error recieving message from server \n");
                }
                printf("%s\n", message_server);
                char optionstr[10];
                fflush(stdout);
                bzero(optionstr, sizeof(optionstr));
                read(0, optionstr, sizeof(optionstr));
                int option=atoi(optionstr);
                //printf("%d\n", option);
                if(send(socked_desc, &option, sizeof(int), 0) <= 0) 
                {
                    perror("[client]Eroare la send() spre server.\n");
                }
                bzero(message_server, sizeof(message_server));
                if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                {
                    perror("[client]Error recieving message from server \n");
                }
                if(strstr(message_server, "selected"))
                    break;
                printf("%s\n", message_server);
                bzero(message_server, sizeof(message_server));
            }
        }
        else if(nr == 1)
        {
            printf ("[client] Bye! :) : %d\n", nr);
            exit_code = 1;
        }
        else if(nr == 0)
        {
            printf ("[client]Wrong digit: try again: %d\n", nr);
            printf("\n");
        }
        else if(nr == 12)
        {
            menuoption = 1;
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                perror("[client]Error recieving message from server \n");
            }
            printf("%s\n", message_server);
            //printf("\n");
        }
        else if(nr == 6) 
        {
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                perror("[client]Error recieving message from server \n");
            }
            printf("%s\n", message_server);
            printf("\n");
        }
        else if(nr == 7) 
        {
            char product_name[255];
            while(1)
            {
                if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                {
                    perror("[client]Error recieving message from server \n");
                }
                printf("%s\n\n", message_server);
                if(strstr(message_server, "not"))
                    break;
                if(strstr(message_server, "no "))
                    break;
                bzero(product_name, sizeof(product_name));
                read(0, product_name, sizeof(product_name));
                if(send(socked_desc, product_name, sizeof(product_name), 0) <= 0) 
                {
                    perror("[client]Eroare la send() spre server.\n");
                    return errno;
                }
                bzero(message_server, sizeof(message_server));
                if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                {
                    perror("[client]Error recieving message from server \n");
                }

                printf("%s\n\n", message_server);
                if(strstr(message_server, "We"))
                {
                    //printf("a intrat in if\n");
                    break;
                }
            }
        }
        else if(nr == 9) 
        {
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                perror("[client]Error recieving message from server \n");
            }
            printf("%s\n", message_server);
        }
        else if(nr == 8) 
        {
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                perror("[client]Error recieving message from server \n");
            }
            printf("%s\n", message_server);
            //printf("\n");
        }
        else if(nr == 10)
        {
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                perror("[client]Error recieving message from server \n");
            }
            printf("%s\n", message_server);
            if(strlen(message_server) < 25)
                printf("nothing here yet :(\n\n");
        }
        else if(nr == 11)
        {
            char product_name[255];
            while(1)
            {
                if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                {
                    perror("[client]Error recieving message from server \n");
                }
                printf("%s\n\n", message_server);
                if(strstr(message_server, "not "))
                    break;
                if(strstr(message_server, "no "))
                    break;
                if(strstr(message_server, "Your"))
                {
                    if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                    {
                        perror("[client]Error recieving message from server \n");
                    }
                    printf("%s\n\n", message_server);
                    if(strstr(message_server, "no "))
                    break;
                }
                bzero(product_name, sizeof(product_name));
                read(0, product_name, sizeof(product_name));
                //write(1, product_name, sizeof(product_name));
                if(send(socked_desc, product_name, sizeof(product_name), 0) <= 0) 
                {
                    perror("[client]Eroare la send() spre server.\n");
                    return errno;
                }
                bzero(message_server, sizeof(message_server));
                if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                {
                    perror("[client]Error recieving message from server \n");
                }

                printf("%s\n", message_server);
                if(strstr(message_server, "We"))
                    break;
            }
        }
        else if(nr == 13)
        {
            char product_name[255];
            printf("Enter the product you are searching :\n");
            product_name[0] = '\0';
            fflush(stdin);
            read(0, product_name, sizeof(product_name));
            //printf("%s\n", product_name);
            if(send(socked_desc, product_name, sizeof(product_name), 0) <= 0) 
            {
                perror("[client]Eroare la send() spre server.\n");
                return errno;
            }
            bzero(message_server, sizeof(message_server));
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                perror("[client]Error recieving message from server \n");
            }

            printf("%s\n\n", message_server);
        }
        else if(nr == 14)
        {
            char product_name[255];
            printf("Enter the market or restaurant you are searching :\n");
            product_name[0] = '\0';
            fflush(stdin);
            read(0, product_name, sizeof(product_name));
            //printf("%s inpit\n", product_name);
            if(send(socked_desc, product_name, sizeof(product_name), 0) <= 0) 
            {
                perror("[client]Eroare la send() spre server.\n");
                return errno;
            }
            message_server[0] = '\0';
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                perror("[client]Error recieving message from server \n");
            }

            printf("%s\n", message_server);
        }
        else if(nr == 15)
        {
            bzero(message_server, sizeof(message_server));
            if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
            {
                perror("[client]Error recieving message from server \n");
            }
            printf("%s\n", message_server);
            char product_name[255];
            product_name[0] = '\0';
            read(0, product_name, sizeof(product_name));
            product_name[strcspn(product_name, "\n")] = '\0';
            //printf("%s inpit\n", product_name);
            if(send(socked_desc, product_name, sizeof(product_name), 0) <= 0) 
            {
                perror("[client]Eroare la send() spre server.\n");
                return errno;
            }
            printf("Enter password (max 39 char): \n");
            memset(message_server, 0, sizeof(message_server));
            memset(product_name, 0, sizeof(product_name));
            while(1)
            {
                read(0, product_name, sizeof(product_name));
                printf("%s\n", product_name);
                if(send(socked_desc, product_name, sizeof(product_name), 0) <= 0) 
                {
                    fprintf(stderr, "[client]Error sending to server \n");
                }
                if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                {
                    perror("[client]Error recieving message from server \n");
                }
                printf("%s\n", message_server);
                if(strstr(message_server, "Your")) 
                {
                    if(recv(socked_desc, message_server, sizeof(message_server), 0) <= 0) 
                    {
                        perror("[client]Error recieving message from server \n");
                    }
                    printf("%s\n", message_server);
                    menuoption = 1;
                    break;
                }   
            }
        }
    }
    close(socked_desc);
    return 0;
}
