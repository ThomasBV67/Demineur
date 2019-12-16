 /**
 * @file main.c  
 * @author Thomas Bureau-Viens
 * @date   16-12-2019
 * @brief  Ce programme est une recréation du jeu démineur sur un écran LCD.
 * Le programme démarre en affichant un écran complet de tuiles. Le curseur du 
 * LDC se déplace grâce au axes X et Y de la manette. Lorsque le bouton est 
 * appuyé, la tuile sous le curseur et toutes celles autour qui ne sont pas des 
 * bombes, soit jusqu'a 8 autres tuiles,sont remplacées par la même position sur
 * le tableau contenant les bombes et des nombres équivalents au nombres de 
 * bombes dans les cases entourant la case du  tableau. La partie se termine, 
 * si il reste le meme nombre de tuiles que de bombes ou si la case sous le 
 * curseur révèle une bombe. 
 *
 * @version 1.0
 * Environnement:
 *     Développement: MPLAB X IDE (version 5.25)
 *     Compilateur: XC8 (version 2.10)
 *     Matériel: Carte démo du Pickit4. PIC 18F45K20
  */

/****************** Liste des INCLUDES ****************************************/
#include <xc.h> // pour le compilateur
#include <stdbool.h>  // pour l'utilisation du type bool
#include <conio.h> // pour kbhit et getch
#include <stdlib.h> // pour srand et rand
#include "Lcd4Lignes.h" // pour l'utilisation du LCD
#include <stdio.h>
#include <string.h>


/********************** CONSTANTES *******************************************/
#define _XTAL_FREQ 1000000 //Constante utilisée par __delay_ms(x). Doit = fréq interne du uC
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 1 //caractère cgram d'une tuile
#define MINE 2 //caractère cgram d'une mine
#define NB_MINE 5


/********************** PROTOTYPES *******************************************/
void initialisation(void);
char getAnalog(char canal);
void initTabVue(void);
void rempliMines(int nb);
void metToucheCombien(void);
char calculToucheCombien(int ligne, int colonne);
void deplace(char* x, char* y);
bool demine(char x, char y);
void enleveTuilesAutour(char x, char y);
bool gagne(int* pMines);
void afficheTabVue(void);
void afficheTabMines(void);

/****************** VARIABLES GLOBALES ****************************************/

 char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau des caractères affichés au LCD
 char m_tabMines[NB_LIGNE][NB_COL+1]={
    "                    ",
    "                    ",
    "                    ",
    "                    "};
 //Tableau contenant les mines, les espaces et les chiffres

//               ***** PROGRAMME PRINCPAL *****   
                           
void main(void)
{
    char posY=1; // Initialisation des variables locales
    char posX=1;
    bool lose=1;
    bool win=0;
    int nbMines=5;
    
    initialisation();  // Initialisation des tableaux et de l'affichage
    lcd_init();
    initTabVue();
    afficheTabVue();
    rempliMines(nbMines);
    metToucheCombien();
    
    lcd_gotoXY(1,1); // Présentation
    lcd_putMessage("Lab6");
    lcd_gotoXY(1,2);
    lcd_putMessage("Thomas Bureau-Viens");
    __delay_ms(3000);
    lcd_effaceAffichage();
    
    while(1) // boucle principale
    {
        deplace(&posX,&posY);
        if(PORT_SW==0)
        {
            while(PORT_SW==0);
            lose = demine(posX-1,posY-1);
            win = gagne(&nbMines);
            if((lose==0)||(win==1)) // si découvre une bombe ou toutes les autres cases retirées, fin de la partie
            {
                afficheTabMines();
                while(PORT_SW==1);
                initTabVue();
                afficheTabVue();
                rempliMines(nbMines);
                metToucheCombien();
            }
            afficheTabVue();
        }
        __delay_ms(100);
    }
}
/*
 * @brief Rempli le tableau m_tabVue avec le caractère spécial (définie en CGRAM
 *  du LCD) TUILE. Met un '\0' à la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    for(int i=0;i<NB_LIGNE;i++)
    {
        for(int j=0;j<NB_COL;j++)
        {   
            m_tabVue[i][j] = TUILE;
        }
        m_tabVue[i][NB_COL]=0;
    }
}

/*
 * @brief Affiche m_tabVue sur le lcd
 * @param rien
 * @return rien
 */
void afficheTabVue(void)
{
    for(int i=0;i<NB_LIGNE;i++)
    {
        lcd_gotoXY(1,i+1);
        lcd_putMessage(m_tabVue[i]);  
    }
}

/*
 * @brief Affiche m_tabMines sur le lcd
 * @param rien
 * @return rien
 */
void afficheTabMines(void)
{
    for(int i=0;i<NB_LIGNE;i++)
    {
        lcd_gotoXY(1,i+1);
        lcd_putMessage(m_tabMines[i]);  
    }
}
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caractère MINE défini en CGRAM.
 * @param int nb, le nombre de mines à mettre dans le tableau 
 * @return rien
 */
void rempliMines(int nb)
{
    char x;
    char y;
    
    for(int i=0;i<NB_LIGNE;i++)
    {
        strcpy(m_tabMines[i],"                    ");  
    }
    for(char i=0;i<nb;i++)
    {
        do
        {
            y=rand()%NB_LIGNE;
            x=rand()%NB_COL;
        }while(m_tabMines[y][x]!=' ');
        m_tabMines[y][x]=MINE;
    }
    
    
}
 
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche à 3 mines, alors la méthode place le code ascii de 3 dans
 * le tableau. Si la case ne touche à aucune mine, la méthode met le code
 * ascii d'un espace.
 * Cette méthode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void)
{
    for(char i=0;i<NB_LIGNE;i++)
    {
        for(char j=0;j<NB_COL;j++)
        {   
            if(m_tabMines[i][j]!=MINE)
            {
                m_tabMines[i][j]=(calculToucheCombien(i,j)+0x30);
                if(m_tabMines[i][j]==0x30)
                {
                    m_tabMines[i][j]=' ';
                }
            }
        }
    }
}

/*
 * @brief Calcul à combien de mines touche la case. Cette méthode est appelée par metToucheCombien()
 * @param int ligne, int colonne La position dans le tableau m_tabMines a vérifier
 * @return char nombre. Le nombre de mines touchées par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    char countMine=0;
    /*for(char i=ligne-1;i<=ligne+1;i++)
    {
        if(i<0)
        {
            i++;
        }
        for(char j=colonne-1;i<=colonne;j++)
        {
            if((m_tabMines[i][colonne-1]==2)&&(ligne-1>=0)&&(colonne-1>=0))
        }
    }*/
    if((m_tabMines[ligne-1][colonne-1]==2)&&(ligne-1>=0)&&(colonne-1>=0))
    {
        countMine++;
    }
    if((m_tabMines[ligne][colonne-1]==2)&&(colonne-1>=0))
    {
        countMine++;
    }
    if((m_tabMines[ligne+1][colonne-1]==2)&&(colonne-1>=0)&&(ligne+1<20))
    {
        countMine++;
    }
    if((m_tabMines[ligne-1][colonne]==2)&&(ligne-1>=0))
    {
        countMine++;
    }
    if((m_tabMines[ligne-1][colonne+1]==2)&&(ligne-1>=0)&&(colonne+1<20))
    {
        countMine++;
    }
    if((m_tabMines[ligne][colonne+1]==2)&&(colonne+1<20))
    {
        countMine++;
    }
    if((m_tabMines[ligne+1][colonne+1]==2)&&(colonne+1<20)&&(ligne+1<20))
    {
        countMine++;
    }
    if((m_tabMines[ligne+1][colonne]==2)&&(ligne+1<20))
    {
        countMine++;
    }
    return countMine;
}

/**
 * @brief Si la manette est vers la droite ou la gauche, on déplace le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y)
{
    if(getAnalog(AXE_X)>150)
    {
        (*x)++;
    }
    if(getAnalog(AXE_X)<100)
    {
        (*x)--;
    }
    if(getAnalog(AXE_Y)>150)
    {
        (*y)--;
    }
    if(getAnalog(AXE_Y)<100)
    {
        (*y)++;
    }
    if(*x<=0)
    {
        *x=NB_COL;
    }
    if(*x>NB_COL)
    {
        *x=1;
    }
    if(*y<=0)
    {
        *y=NB_LIGNE;
    }
    if(*y>NB_LIGNE)
    {
        *y=1;
    }
    lcd_gotoXY(*x,*y);
} 

/*
 * @brief Dévoile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derrière les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    if(m_tabMines[y][x]== MINE)
    {
        return false;
    }
    if(m_tabVue[y][x]==TUILE)
    {
        enleveTuilesAutour(x,y);
    }
    m_tabVue[y][x] = m_tabMines[y][x];
    return true;
}

/*
 * @brief Dévoile les cases non minées autour de la tuile reçue en paramètre.
 * Cette méthode est appelée par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    if((m_tabMines[y-1][x-1]!=MINE)&&(x>=0)&&(y>=0))
    {
        m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];
    }
    if((m_tabMines[y-1][x]!=MINE)&&(y>=0))
    {
        m_tabVue[y-1][x] = m_tabMines[y-1][x];  
    }
    if((m_tabMines[y-1][x+1]!=MINE)&&(y>=0)&&(x<NB_COL))
    {
        m_tabVue[y-1][x+1] = m_tabMines[y-1][x+1];
    }
    if((m_tabMines[y+1][x-1]!=MINE)&&(x>=0)&&(y<NB_LIGNE))
    {
        m_tabVue[y+1][x-1] = m_tabMines[y+1][x-1];
    }
    if((m_tabMines[y][x-1]!=MINE)&&(x>=0))
    {
        m_tabVue[y][x-1] = m_tabMines[y][x-1];
    }
    if((m_tabMines[y+1][x]!=MINE)&&(y<NB_LIGNE))
    {
        m_tabVue[y+1][x] = m_tabMines[y+1][x];
    }
    if((m_tabMines[y+1][x+1]!=MINE)&&(y<NB_LIGNE)&&(x<NB_COL))
    {
        m_tabVue[y+1][x+1] = m_tabMines[y+1][x+1];
    }
    if((m_tabMines[y][x+1]!=MINE)&&(x<NB_COL))
    {
        m_tabVue[y][x+1] = m_tabMines[y][x+1];
    }
}

/*
 * @brief Vérifie si gagné. On a gagné quand le nombre de tuiles non dévoilées
 * est égal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagné.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagné, faux sinon
 */
bool gagne(int* pMines)
{
    char count=0; 
    for(char i=0;i<NB_LIGNE;i++)
    {
        for(char j=0;j<NB_COL;j++)
        {
            if(m_tabVue[i][j]==TUILE)
            {
                count++;
            }
        }
    }
    if(count == *pMines)
    {
        (*pMines)++;
        return true;  
    }
    else
    {    
        return false;
    }
}

/*
 * @brief Lit le port analogique. 
 * @param Le no du port à lire
 * @return La valeur des 8 bits de poids forts du port analogique
 */
char getAnalog(char canal)
{ 
    ADCON0bits.CHS = canal;
    __delay_us(1);  
    ADCON0bits.GO_DONE = 1;  //lance une conversion
    while (ADCON0bits.GO_DONE == 1) //attend fin de la conversion
        ;
    return  ADRESH; //retourne seulement les 8 MSB. On laisse tomber les 2 LSB de ADRESL
}
 
/*
 * @brief Fait l'initialisation des différents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 à RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN à on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement à gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB à gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fréquence pour la conversion la plus longue possible)
 
 
    /**************Timer 0*****************/
    T0CONbits.TMR0ON    = 1;
    T0CONbits.T08BIT    = 0; // mode 16 bits
    T0CONbits.T0CS      = 0;
    T0CONbits.PSA       = 0; // prescaler enabled
    T0CONbits.T0PS      = 0b010; // 1:8 pre-scaler
    INTCONbits.TMR0IE   = 1;  // timer 0 interrupt enable
    INTCONbits.TMR0IF   = 0; // timer 0 interrupt flag
    INTCONbits.PEIE = 1; //permet interruption des périphériques
    INTCONbits.GIE = 1;  //interruptions globales permises
}