// Boink
// http://wab.com/?screen=156

// face 0 = boink : module Enigma
// face 1 = fullscreen TCB : tcbfull.wav
// face 2 = sprites TCB : YM : so watt - tcb sprites.ym
// face 3 = Megascroll ULM : ULM.wav
// face 4 = TechTech : mod ronken.mod
// face 5 = SOTB : beast.ym



// OK trier les faces
// KO verifier visibilité de chaque face
// OK remplir 

// OK Reduire à 600*600
// OK copier dans framebuffer

// OK passer dans le cube comme texture
// KO : tester % de CPU en utilsant un pbo : https://riptutorial.com/opengl/example/28872/using-pbos
// OK : identifier les faces visibles en fonction de pos_rotation x , y et z : calculs GLM
// KO ré organiser : faire boink après 3D
// OK ajouter le player de module
// OK : inutile : ne generer l'affichage que quand la face est visible
// ré écrire Boink en open gl

#define  MONPI  3.14159265358979323846

int numero_mod=0;
float luminosite;


// Define the desired framebuffer resolution (here we set it to 720p).
#define FB_WIDTH  1280
#define FB_HEIGHT 720


#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <sys/errno.h>
#include <climits>
#include <cstdlib>

#include <switch.h>
#include <threads.h>

#include "stb_image.h"

// outtro
// #define couleur_fond 0xFFFFFFFF
#define couleur_fond 0x00000000
//#define largeur_texture 1280+64+64 = 1408
#define largeur_texture_outtro 1600
#define bord_gauche_texture_outtro 120
//#define hauteur_texture_outtro 720+64+64
#define hauteur_texture_outtro 848
#define bord_haut_texture_outtro 64



u32* buffer_ecran_outtro;
#include "32x32_png.h"
#define largeur_32x32_outtro 320
#define hauteur_32x32_outtro 192
stbi_uc* 	pointeur_font_outtro;
u32* 		pointeur_font_outtro_u32;

char* TEXTE_outtro = "\
          CREDITS FOR THIS DEMO:         \
                    ---                  \
ALL WORK BASED ON LIBNX                  \
A LOT OF CODE FROM THE EXAMPLES          \
PROVIDED WITH THE LIBRARY-MANY THANKS !  \
HXCMOD PLAYER BY JEAN FRANCOIS DEL NERO  \
ST-SOUND YM PLAYER BY ARNAUD CARRE       \
                                         \
LOTS OF STUFF FROM THE WAB-COM TEAM      \
MISSING OR WRONG DATAS RIPPED FROM ST    \
USING STEEM SSE BY STEVEN SEAGAL         \
                                         \
YM CHIPTUNES BY JOCHEN HIPPEL            \
RONKEN MODULE BY BLAIZER                 \
ENIGMA MODULE BY FIREFOX AND TIP         \
THIS MODULE BY MOBY AKA ELMOBO           \
( BEST MODULE EVER !)                    \
                                         \
ALL GRAPHICS FROM THEIR ORIGINAL AUTHORS \
THIS FONT BY THE SERGEANT (BUSHWACKERS)  \
                                         \
HAVE YOU TRIED PRESSING THE KEYS ?       \
                                         \
                ATARI RULEZZZZ SINCE 1985$";


// intro
int	diviseur_volume_global=1;
#define position_x_logo_nintendo 460
#define position_y_logo_nintendo 180

#define x_min_fenetre_recherche_de_point	400
#define y_min_fenetre_recherche_de_point	0
#define x_max_fenetre_recherche_de_point	880
#define y_max_fenetre_recherche_de_point	719
#define largeur_fenetre_recherche_de_point x_max_fenetre_recherche_de_point-x_min_fenetre_recherche_de_point
#define hauteur_fenetre_recherche_de_point y_max_fenetre_recherche_de_point-y_min_fenetre_recherche_de_point


u32* buffer_ecran_source;
u32* buffer_ecran_dest;

#include "nintendo2_png.h"
#define largeur_logo_Nintendo 360
#define hauteur_logo_Nintendo 360
stbi_uc* 	pointeur_logo_nintendo;
u32* 		pointeur_logo_Nintendo_u32;


#define nombre_de_flocons	8000
#define nombre_de_floconsmaxi 20000

int nombre_pixels_logo_Nintendo=0;
int nombre_points_actifs=0;
int ca_a_commence=0;
int c_est_termine=0;
int sequence_logo_Atari_terminee=0;
int prochaine_recherche_logo_Atari_lineaire=0;

struct flocon 
{
	int		posx;
	int		posy;
	int		vitesse;
	int		delai_avant_dispo;
	int		actif;
	int		posx_destination_finale;
	int		posy_destination_finale;
	u32		couleur_point_precedent;
	float	incrementx;
	float	incrementy;
	float	posx_actuelle_avec_virgule;
	float	posy_actuelle_avec_virgule;
	int		nb_etapes;
	int		point_source_est_reutilise;
	int		n_a_pas_demarre;
	u32		couleur_finale;
	
};

struct flocon flocons[nombre_de_floconsmaxi];
struct flocon points_logo_Atari[20000];
struct flocon points_logo_detruit[20000];



int min_y=FB_HEIGHT;
int min_y_precedent=0;

#define position_x_logo_Atari 460
#define position_y_logo_Atari 0

#include "atari_png.h"
#define largeur_logo_Atari 360
#define hauteur_logo_Atari 258
stbi_uc* 	pointeur_logo_Atari;
u32* 		pointeur_logo_Atari_u32;

#include "seven_ym_bin.h"

#include "union_ym_bin.h"
int numero_musique=1;



//////////////////////////////// fin intro

typedef struct
{
    float position[3];
    float texcoord[2];
    float normal[3];
} Vertex;

static const Vertex vertex_list[] =
{
// coordonnees cube
// Boink en 720x720
    // First face (PZ) Front
    // First triangle y=0-0.125
    { {-0.5f, -0.5f, +0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, +1.0f} },
    { {+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, +1.0f} },
    { {+0.5f, +0.5f, +0.5f}, {1.0f, 0.125f}, {0.0f, 0.0f, +1.0f} },
    // Second triangle
    { {+0.5f, +0.5f, +0.5f}, {1.0f, 0.125f}, {0.0f, 0.0f, +1.0f} },
    { {-0.5f, +0.5f, +0.5f}, {0.0f, 0.125f}, {0.0f, 0.0f, +1.0f} },
    { {-0.5f, -0.5f, +0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, +1.0f} },

// TCB Fullscreen
    // Second face (MZ) Back
    // First triangle y=0.125-0.250
    { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.173f}, {0.0f, 0.0f, -1.0f} },
    { {-0.5f, +0.5f, -0.5f}, {0.58f, 0.173f}, {0.0f, 0.0f, -1.0f} },
    { {+0.5f, +0.5f, -0.5f}, {0.58f, 0.125f}, {0.0f, 0.0f, -1.0f} },
    // Second triangle
    { {+0.5f, +0.5f, -0.5f}, {0.58f, 0.125f }, {0.0f, 0.0f, -1.0f} },
    { {+0.5f, -0.5f, -0.5f}, {0.0f, 0.125f }, {0.0f, 0.0f, -1.0f} },
    { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.173f}, {0.0f, 0.0f, -1.0f} },

// TCB sprites 320x200
    // Third face (PX)
    // First triangle y=0.250-0.375
    { {+0.5f, -0.5f, -0.5f}, {0.444444444f, 0.250f}, {+1.0f, 0.0f, 0.0f} },
    { {+0.5f, +0.5f, -0.5f}, {0.0f, 0.250f}, {+1.0f, 0.0f, 0.0f} },
    { {+0.5f, +0.5f, +0.5f}, {0.0f, 0.284722222f}, {+1.0f, 0.0f, 0.0f} },
    // Second triangle
    { {+0.5f, +0.5f, +0.5f}, {0.0f, 0.284722222f}, {+1.0f, 0.0f, 0.0f} },
    { {+0.5f, -0.5f, +0.5f}, {0.444444444f, 0.284722222f}, {+1.0f, 0.0f, 0.0f} },
    { {+0.5f, -0.5f, -0.5f}, {0.444444444f, 0.250f}, {+1.0f, 0.0f, 0.0f} },
// megascroll ULM 384 x 264
// 0.421 ?
    // Fourth face (MX)
    // First triangle y=0.375-0.5
    
    // First triangle y=0.375-0.5
    { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.421f}, {-1.0f, 0.0f, 0.0f} },
    { {-0.5f, -0.5f, +0.5f}, {0.5333334f, 0.421f}, {-1.0f, 0.0f, 0.0f} },
    { {-0.5f, +0.5f, +0.5f}, {0.5333334f, 0.375f}, {-1.0f, 0.0f, 0.0f} },
    // Second triangle
    { {-0.5f, +0.5f, +0.5f}, {0.5333334f, 0.375f}, {-1.0f, 0.0f, 0.0f} },
    { {-0.5f, +0.5f, -0.5f}, {0.0f, 0.375f}, {-1.0f, 0.0f, 0.0f} },
    { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.421f}, {-1.0f, 0.0f, 0.0f} },

// TechTech
    // Fifth face (PY)
    // First triangle  y=0.5-0.625
    { {-0.5f, +0.5f, -0.5f}, {0.444444444f, 0.5f}, {0.0f, +1.0f, 0.0f} },
    { {-0.5f, +0.5f, +0.5f}, {0.0f, 0.5f}, {0.0f, +1.0f, 0.0f} },
    { {+0.5f, +0.5f, +0.5f}, {0.0f, 0.53472222f}, {0.0f, +1.0f, 0.0f} },
    // Second triangle
    { {+0.5f, +0.5f, +0.5f}, {0.0f, 0.53472222f}, {0.0f, +1.0f, 0.0f} },
    { {+0.5f, +0.5f, -0.5f}, {0.444444444f, 0.53472222f}, {0.0f, +1.0f, 0.0f} },
    { {-0.5f, +0.5f, -0.5f}, {0.444444444f, 0.5f}, {0.0f, +1.0f, 0.0f} },

// Shadow of the Beast
    // Sixth face (MY)
    // First triangle  y=0.625-0.75
    { {-0.5f, -0.5f, -0.5f}, {0.577777f, 0.625f}, {0.0f, -1.0f, 0.0f} },
    { {+0.5f, -0.5f, -0.5f}, {0.0f, 0.625f}, {0.0f, -1.0f, 0.0f} },
    { {+0.5f, -0.5f, +0.5f}, {0.0f, 0.671875f}, {0.0f, -1.0f, 0.0f} },
    // Second triangle
    { {+0.5f, -0.5f, +0.5f}, {0.0f, 0.671875f}, {0.0f, -1.0f, 0.0f} },
    { {-0.5f, -0.5f, +0.5f}, {0.577777f, 0.671875f}, {0.0f, -1.0f, 0.0f} },
    { {-0.5f, -0.5f, -0.5f}, {0.577777f, 0.625f}, {0.0f, -1.0f, 0.0f} },
};

#define vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))


int mode_manuel=0;
int face_la_plus_visible=0;
float normal_max=0;

#include "knulla_mod_bin.h"
// MOD data
// extern const unsigned char mod_data[39424];

#include "enigma_mod_bin.h"
// enigma_mod_bin[]
// enigma_mod_bin_end[]
// enigma_mod_bin_size

#include "ronken_mod_bin.h"
#include "beast_ym_bin.h"
#include "tcbsprites_ym_bin.h"
#include "tcbfull_ym_bin.h"
#include "ULMmegascroll_ym_bin.h"


// Init TechTech

	int		x_fond_rose_TechTech=0;
	int		y_fond_rose_TechTech=0;
	
	int		x_fond_vert_TechTech=0;
	int		y_fond_vert_TechTech=50;
	
	int	increment_fond_rose_x = -16;
	int	increment_fond_rose_y = 0;
	
	int	increment_fond_vert_x = 0;
	int	increment_fond_vert_y = -2;

#define largeur_TechTech 320
#define hauteur_TechTech 200
u32*	buffer_ecran_TechTech;

stbi_uc* 	pointeur_fond_rose_TechTech;
u32* 		pointeur_fond_rose_TechTech_u32;

#include	"Grodan_pink_png.h"
// fond rose en png
// 320x200
#define		largeur_fond_rose_TechTech	320
#define		hauteur_fond_rose_TechTech	200


stbi_uc* 	pointeur_fond_vert_TechTech;
u32* 		pointeur_fond_vert_TechTech_u32;

#include	"Grodan_green_png.h"
// fond rose en png
// 320x200
#define		largeur_fond_vert_TechTech	320
#define		hauteur_fond_vert_TechTech	200


// pour les sprites:
// largeur = 17 dans le png, mais utilisé = 16 
// donc sprite à x = 0,17,34,etc
// hauteur = 10

#include	"spriteTech_png.h"

#define nombre_sprites_TechTech 12
#define largeur_bande_sprite_TechTech 204
// 12 sprites * 17 de largeur
#define largeur_sprite_TechTech 16
#define hauteur_sprite_TechTech 10
#define ecart_entre_sprite_dans_png_TechTech 17

stbi_uc* 	pointeur_sprites_TechTech;
u32* 		pointeur_sprites_TechTech_u32;

float ychange = 0.0f;
float swing = 0.0f;
float swingy = 0.0f;
int spx = 152;
int spy = 50;
float addy = 0.1f;
float siny = 0.0f;
int x_sprite_en_cours;
int y_sprite_en_cours;



// scrolling du bas
// 
#include "bsfont9696_png.h"
// bsfont_png
#include "bigscrollraster_png.h"
// bigscrollraster_png
#define largeur_caractere_scrolling_bas_TechTech	96
#define hauteur_caractere_scrolling_bas_TechTech	96

stbi_uc* 	pointeur_fonte_scrolling_bas_TechTech;
u32* 		pointeur_fonte_scrolling_bas_TechTech_u32;

#define hauteur_rasters_scrolling_bas_TechTech	96
#define largeur_caractereraster_scrolling_bas_TechTech 96
#define hauteur_caractereraster_scrolling_bas_TechTech 96

stbi_uc* 	pointeur_rasters_scrolling_bas_TechTech;
u32* 		pointeur_rasters_scrolling_bas_TechTech_u32;

u32*		buffer_fonteraster_scrolling_bas_TechTech;
int		pos_dans_fonte_scrolling_bas_TechTech;
u32*	pointeur_pos_caractere_en_cours_scrolling_bas_TechTech=0;
int		pos_dans_texte_scrolling_bas_TechTech=0;


char* text_scrolltext_bas_TechTech = "                               HI AND WELCOME TO THE GRODAN AND KVACK KVACK DEMO (THAT NAME WILL PROBABLY MAKE US FAMOUS IN THE GUINNESS BOOK OF RECORDS - THE MOST STUPID NAME IN DEMO HISTORY.  THE PREVIOUS POSSESSORS OF THAT RECORD WAS OMEGA WITH -OMEGAKUL-.   I'M AFRAID WE WILL SOON BE BEATEN BY SYNC'S 'MJOFFE-DEMO', WITH TWO DOTS ABOVE THE 'O'.  DID YOU KNOW THAT THIS IS A COMMENT IN THE MIDDLE OF A SENTENCE? NO?  WE ALSO FORGOT, BUT LET'S CONTINUE WITH WHAT WE WERE WRITING BEFORE WE STARTED WRITING THIS RECORD-CRAP.), CODED BY NICK AND JAS OF THE CAREBEARS. GRAPHIXXXX BY TANIS, THE GREAT (?) OF THE MEGAMIGHTY CAREBEARS.        WE HAVE TO COVER TWO SUBJECTS IN THIS SCROLLTEXT - THE FANTASTIC WORLD OF HARDWARESCROLLERS  AND  GREETINGS....   LET'S START WITH THE STUFF YOU PROBABLY WANT US TO TALK THE MOST ABOUT - HARDWARESCROLLERS....        TIME: LATE MARCH 1989    PLACE: NICK'S COMPUTER ROOM     IT WORKS!!!!!!!  AFTER HAVING TRIED THE ZANY SCROLLTECHNIQUE ON BOTH NICK'S AND JAS' COMPUTERS, WE CONCLUDED THAT IT ACTUALLY WORKED.    ONE DAY LATER, OMEGA CALLS US AND GOES SOMETHING LIKE THIS: - HAAAA HAAAA  WE KNOW HOW TO SCROLL THE WHOLE SCREEN BOTH HORIZONTALLY AND VERTICALLY IN LESS THAN TEN SCANLINES!!!!!!         WE WERE AMAZED THAT THEY HAD ACTUALLY COME UP WITH THE SAME IDEA ON THE SAME DAY AS US, BUT AT LEAST NOBODY ELSE KNEW HOW TO DO IT.     WE MANAGED TO RELEASE THE FIRST HARDWARESCROLLER THE WORLD HAS SEEN, IN THE CUDDLY DEMOS, AND NOW WE ARE GOING TO USE IT COMERCIALLY (CODING GAMES, DICKHEAD)....     NOW A HINT HOW IT'S DONE:    IT HAS NOTHING TO DO WITH ANY OF THE SOUND-REGISTERS.....         HERE IS ANOTHER ADDRESS TO THE CAREBEARS:     T H E   C A R E B E A R S ,    D R A K E N B E R G S G   2 3    8 T R ,      1 1 7   4  1   S T O  C K H O L M ,     S W E  D E N .                NOW FOR SOME GREETINGS:   MEGADUNDERSUPERDUPERGREETINGS TO  ALL THE OTHER MEMBERS OF THE UNION, ESPECIALLY THE EXCEPTIONS (TANIS WISH TO GIVE A SPECIAL HI TO ES) AND THE REPLICANTS (GOODBYE, RATBOY! YOUR INTROS WERE GREAT).   NORMAL MEGAGREETINGS (IN MERIT-ORDER)(WOW) TO   SYNC (WE'VE CHANGED OUR MINDS, YOU'RE THE SECOND BEST SWEDISH CREW. WE JUST HADN'T SEEN MANY SCREENS BY YOU GUYS (IT'S UNDERSTANDABLE - YOU HAVE ONLY RELEASED THREE NOT VERY GOOD ONES)),  OMEGA (TOO BAD, YOU'RE NOT THE SECOND BEST ANYMORE.  PERHAPS IT HAS SOMETHING TO DO WITH  THE TERA-DISTER, THE 'TCB-E'-JATTEDUMMA'-SIGN OR THE FACT THAT SYNC IS BETTER), THE LOST BOYS (SEE YA' SOON AND WE'RE ANXIOUSLY AWAITING YOUR MEGAMEGADEMO)             SOMETHING BETWEEN MEGAGREETINGS AND NORMAL GREETINGS TO:   FLEXIBLE FRONT (GOODBYE), VECTOR (SO YOU CRACKED OUR DEMO, HUH? NICE SCREEN, BY THE WAY), GHOST (SO YOU TRIED TO CRACK OUR DEMO, HUH? GREAT SCREEN, BY THE WAY), 2 LIFE CREW (YOU ARE IMPROVING), MAGNUM FORCE (YOU SEEM TO BE THE BEST OPTIMIZERS IN FRANCE!), NORDIK CODERS (NICE SCREEN).   NORMAL GREETINGS TO:  FASHION (GOOD LUCK WITH YOUR DEMO), OVERLANDERS (THANKS FOR NOT INCLUDING CUDDLY IN YOUR DEMOBREAKER), NO CREW (ESPECIALLY ROCCO. YOU ARE IMPROVING), AUTOMATION (GREAT COMPACT DISKS), MEDWAY BOYS (NICE CD'S),  ST CONNEXION (HOPE YOUR DEMO WILL BE AS GOOD AS YOUR GRAPHICS), FOXX (COOL SCREEN), FOFT (KEEP ON COMPACTING), ZAE (WE HAD A GREAT TIME IN MARSEILLE), KREATORS (ESPECIALLY CHUD), M.A.R.K.U.S (PLEASE SPREAD THIS DEMO AS MUCH AS YOU SPREAD CUDDLY DEMOS), HACKATARIMAN (THANKS FOR ALL THE STUFF), THE ALLIANCE (ESPECIALLY OVERLANDERS (THANKS FOR TCB-FRIENDLY SCROLLTEXTS AND MANY NICE SCREENS), AND BLACK MONOLITH TEAM (YOUR DEMOSCREEN WAS THE BEST IN THE OLD ALLIANCE DEMO), BIRDY (SEND US YOUR CRACKS), LINKAN 'THE LINK' 'JUDGE LINK' LINKSSON (PING-PONG), NYARLOTHATEPS ADEPTS (STRANGE NAME, STRANGE GUYS), GROWTWIG ( NO COMMENT),  TONY KOLLBERG (TJENA, LYCKA TILL MED ASSEMBLERN)     END OF GREETINGS. IF YOU WERE NOT GREETED, TOO BAD. NORMAL FUCKING GREETINGS TO:  CONSTELLATIONS (NOONE WILL EVER COMPLAIN ABOUT TCB AND GET AWAY WITH IT, BESIDES YOUR DEMO WAS WORTHLESS). MEGA FUCKING GREETINGS TO:     MENACING CRACKING ALLIANCE (SO, YOU DON'T LIKE BEING CALLED LAMERS, HOW YA' LIKE BEING CALLED:       MOTHERFUCKIN'   BLEEDIN' (BRITTISH ENGLISH) ULTIMATE CHICKENBRAINS????!!!! I BET IT'S ALMOST AS FUN AS FUCKING GREET TCB).  END OF SCROLLTEXT. LET'S WRAP.$";

// scrollings verticaux
#include "fontverticale1615_png.h"
// fontverticale_png
// largeur = 16
// hauteur = 14

#define hauteur_caractere_scrolling_vertical_TechTech 15
#define largeur_caractere_scrolling_vertical_TechTech 16
stbi_uc* 	pointeur_fonte_scrolling_vertical_TechTech;
u32*	pointeur_fonte_scrolling_vertical_TechTech_u32;

int		pos_dans_fonte_scrolling_vertical_TechTech=0;
u32*	pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech=0;
int		pos_dans_texte_scrolling_vertical_TechTech=0;


char* text_scrolltext_vertical_TechTech = "                           TANIS, THE FAMOUS GRAFIXX-MAN, IS A NEW MEMBER OF TCB.  HE MADE ALL THE GRAPHICS IN THIS SCREEN PLUS LOTSA LOGOS IN THE MAIN MENU.  WE AGREE THAT THIS gONE-BIT-PLANE-MANIAg DOESNhT LOOK VERY GOOD, BUT IT HAD TO BE DONE BY SOMEONE........   BAD LUCK FOR TANIS THAT WE WONhT MAKE MORE DEMOS, THOUGH....       9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9  ..................                 LEThS WRAP (WE SPELLED IT CORRECTLY!!!).......   $";

u32 tableau_rasters_verticaux_TechTech[] = {0xFF0000E0,0xFF0000E0,0xFF0000E0,0xFF0000E0,0xFF2000E0,0xFF2000E0,0xFF4000E0,0xFF4000E0,0xFF6000E0,0xFF6000E0,0xFF8000E0,0xFF8000E0,0xFFA000E0,0xFFA000E0,0xFFC000E0,0xFFC000E0,0xFFE000E0,0xFFE000E0,0xFFE020E0,0xFFE020E0,0xFFE040E0,0xFFE040E0,0xFFE060E0,0xFFE060E0,0xFFE080E0,0xFFE080E0,0xFFE0A0E0,0xFFE0A0E0,0xFFE0C0E0,0xFFE0C0E0,0xFFE0E0E0,0xFFE0E0E0,0xFFC0E0E0,0xFFC0E0E0,0xFFA0E0E0,0xFFA0E0E0,0xFF80E0E0,0xFF80E0E0,0xFF60E0E0,0xFF60E0E0,0xFF40E0E0,0xFF40E0E0,0xFF20E0E0,0xFF20E0E0,0xFF00E0E0,0xFF00E0E0,0xFF00E0C0,0xFF00E0C0,0xFF00E0A0,0xFF00E0A0,0xFF00E080,0xFF00E080,0xFF00E060,0xFF00E060,0xFF00E040,0xFF00E040,0xFF00E020,0xFF00E020,0xFF00E000,0xFF00E000,0xFF20E000,0xFF20E000,0xFF40E000,0xFF40E000,0xFF60E000,0xFF60E000,0xFF80E000,0xFF80E000,0xFFA0E000,0xFFA0E000,0xFFC0E000,0xFFC0E000,0xFFE0E000,0xFFE0E000,0xFFE0E020,0xFFE0E020,0xFFE0E040,0xFFE0E040,0xFFE0E060,0xFFE0E060,0xFFE0E080,0xFFE0E080,0xFFE0E0A0,0xFFE0E0A0,0xFFE0E0C0,0xFFE0E0C0,0xFFE0E0E0,0xFFE0E0E0,0xFFE0C0C0,0xFFE0C0C0,0xFFE0A0A0,0xFFE0A0A0,0xFFE08080,0xFFE08080,0xFFE06060,0xFFE06060,0xFFE04040,0xFFE04040,0xFFE02020,0xFFE02020,0xFFE00000,0xFFE00000,0xFFE02000,0xFFE02000,0xFFE04000,0xFFE04000,0xFFE06000,0xFFE06000,0xFFE08000,0xFFE08000,0xFFE08000,0xFFE08000,0xFFE0A000,0xFFE0A000,0xFFE0C000,0xFFE0C000,0xFFE0E000,0xFFE0E000,0xFFE0E020,0xFFE0E020,0xFFE0E040,0xFFE0E040,0xFFE0E060,0xFFE0E060,0xFFE0E080,0xFFE0E080,0xFFE0E0A0,0xFFE0E0A0,0xFFE0E0C0,0xFFE0E0C0,0xFFE0E0E0,0xFFE0E0E0,0xFFE0C0E0,0xFFE0C0E0,0xFFE0A0E0,0xFFE0A0E0,0xFFE080E0,0xFFE080E0,0xFFE060E0,0xFFE060E0,0xFFE040E0,0xFFE040E0,0xFFE020E0,0xFFE020E0,0xFFE000E0,0xFFE000E0,0xFFC000E0,0xFFC000E0,0xFFA000E0,0xFFA000E0,0xFF8000E0,0xFF8000E0,0xFF6000E0,0xFF6000E0,0xFF4000E0,0xFF4000E0,0xFF2000E0,0xFF2000E0,0xFF0000E0,0xFF0000E0,0xFF0020E0,0xFF0020E0,0xFF0040E0,0xFF0040E0,0xFF0060E0,0xFF0060E0,0xFF0080E0,0xFF0080E0,0xFF00A0E0,0xFF00A0E0,0xFF00C0E0,0xFF00C0E0,0xFF00E0E0,0xFF00E0E0,0xFF00E0C0,0xFF00E0C0,0xFF00E0A0,0xFF00E0A0,0xFF00E080,0xFF00E080,0xFF00E060,0xFF00E060,0xFF00E040,0xFF00E040,0xFF00E020,0xFF00E020,0xFF00E000,0xFF00E000,0xFF20E000,0xFF20E000,0xFF40E000,0xFF40E000,0xFF60E000,0xFF60E000,0xFF80E000,0xFF80E000,0xFFA0E000,0xFFA0E000,0xFFA0E000,0xFFA0E000};

// scrollings 8x8
#include "lfont_png.h"
// largeur = 8
// hauteur = 8
#define hauteur_caractere_scrolling_8x8_TechTech 8
#define largeur_caractere_scrolling_8x8_TechTech 8
stbi_uc* 	pointeur_fonte_scrolling_8x8_TechTech;
u32*	pointeur_fonte_scrolling_8x8_TechTech_u32;

#define position_verticale_scrolling_8x8h_TechTech 4
int		pos_dans_fonte_scrolling_8x8h_TechTech=0;
u32*	pointeur_pos_caractere_en_cours_scrolling_8x8h_TechTech=0;
int		pos_dans_texte_scrolling_8x8h_TechTech=0;

#define position_verticale_scrolling_8x8b_TechTech 31

int		pos_dans_fonte_scrolling_8x8b_TechTech=0;
u32*	pointeur_pos_caractere_en_cours_scrolling_8x8b_TechTech=0;
int		pos_dans_texte_scrolling_8x8b_TechTech=0;

char* text_scrolltext_8x8h_TechTech = "                                                        ONCE UPON A TIME, WHEN THE JUNK DEMO WAS ALMOST FINISHED - WHEN THE BEST DEMO ON THE ST-MARKET WAS 'LCD' BY TEX, WE VISITED IQ2-CREW (AMIGA-FREAKS). THEY SHOWED US A COUPLE OF DEMOS AND ONE OF THEM WAS THE TECHTECH-DEMO BY SODAN AND MAGICIAN 42. KRILLE AND PUTTE LAUGHED AT US AND SAID THAT IT WAS TOTALLY IMPOSSIBLE TO MAKE ON AN ST. WE STUDIED IT FOR HALF AN HOUR AND SAID: -OF COURSE IT'S POSSIBLE.   WHEN WE WERE BACK HOME (WHEN NO AMIGA-OWNER WAS LISTENING), WE CONCLUDED THAT THERE WAS SIMPLY TOO MUCH MOVEMENT FOR AN ST.        NOW, WE HAVE CONVERTED IT ANYWAY. THE AMIGA VERSION HAD SOME UGLY LINES WHIZZING AROUND, BUT WE HAVE 3 VOICE REAL DIGISOUND AND SOME UGLY SPRITES. BESIDES, WE HAVE SOME TERRIBLE RASTERS.......            WE AGREE THAT THERE ARE BETTER AMIGA-DEMOS NOW, AND PERHAPS WE WILL CONVERT SOME MORE IN THE FUTURE.......     LET'S WRAZZZZZZZ................$";

char* text_scrolltext_8x8b_TechTech = "                               EVERYBODY THOUGHT IT WAS IMPOSSIBLE.....                                     EVEN WE THOUGHT IT WAS aIMPOSSIBLE......                                       IT'S A PITY IT WASN'T.....b                                                 THE CAREBEARS PRESENT THE UGLIEST DEMO SO FARc - THE GRODAN AND KVACK KVACK DEMO, A CONVERSIONd OF THE STUNNING TECHTECH DEMO BY SODAN AND MAGICIAN 42 (ON THE COMPUTER THAT CRASHES WHEN YOU ENTER SUPERVISOR MODE IN SEKA).   IT WAS UGLY ON THE AMIGA TOO, BUT IT SURE KNOCKED YOU OFF THE CHAIR WHEN YOU SAW IT THE FIRST TIME.    $";


// timing
int flag_a = 0;
int flag_b = 0;
int flag_c = 0;
int flag_d = 0;






extern "C" {
#include "../source/hxcmod.h"
}

#include "StSoundLibrary.h"


#define SAMPLERATE 48000
#define CHANNELCOUNT 2
#define FRAMERATE 60
#define SAMPLECOUNT (SAMPLERATE / FRAMERATE)
#define BYTESPERSAMPLE 2
#define HXCMOD_UNSIGNED_OUTPUT 1
#define HXCMOD_BIGENDIAN_MACHINE 0
#define SAMPLE_PER_BUF SAMPLECOUNT*2

#define SAMPLERATEYM 44100
#define CHANNELCOUNTYM 1
#define FRAMERATEYM 60
#define SAMPLECOUNTYM (SAMPLERATEYM / FRAMERATEYM)
#define BYTESPERSAMPLEYM 2
#define SAMPLE_PER_BUFYM SAMPLECOUNTYM*2

AudioOutBuffer audout_buf1;
AudioOutBuffer audout_buf2;
AudioOutBuffer *pointeur_buffer1;
AudioOutBuffer *pointeur_buffer2;
AudioOutBuffer *pointeur_buffertmp;
AudioOutBuffer *audout_released_buf;

modcontext* modctx_enigma;
modcontext* modctx_ronken;
modcontext* modctx_knulla;
YMMUSIC* pMusic_beast;
YMMUSIC* pMusic_spritesTCB;
YMMUSIC* pMusic_TCBsample;
YMMUSIC* pMusic_ULM;


unsigned char* pointeur_module_YM_beast;
int taille_module_YM_beast;
unsigned char* pointeur_module_YM_spritesTCB;
int taille_module_YM_spritesTCB;
unsigned char* pointeur_module_YM_TCBsample;
int taille_module_YM_TCBsample;
unsigned char* pointeur_module_YM_ULM;
int taille_module_YM_ULM;

Result rc = 0;
u32 nbbufrelease;

static bool running = true;
static bool runningIntro = true;

#define GLM_FORCE_SWIZZLE 1

#include <EGL/egl.h>    // EGL library
#include <EGL/eglext.h> // EGL extensions
#include <glad/glad.h>  // glad library (OpenGL loader)


// GLM headers
#define GLM_FORCE_PURE
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#define feedback_visibilite 1
#ifdef feedback_visibilite
float feedback[36];
#endif

static GLint loc_mdlvMtx, loc_projMtx;
static GLint loc_lightPos, loc_ambient, loc_diffuse, loc_specular, loc_tex_diffuse0,loc_tex_diffuse1;
glm::mat4 mdlvMtx{1.0};
    
static u64 s_startTicks;

int face0visible=0, face1visible=0, face2visible=0, face3visible=0, face4visible=0, face5visible=0;

constexpr auto TAU = glm::two_pi<float>();

int width, height, nchan;

float pos_rotation_x,pos_rotation_y,pos_rotation_z;
float angle_x_encours=0, angle_y_encours=0 ,angle_z_encours=0;

// parametres du screen demo TCB
int attente_demarrage_fullscreen_TCB = 25*60;

#define nb_ecrans_fullscreen_TCB 8
#define largeur_fullscreen_TCB 416
#define hauteur_fullscreen_TCB 276

#define ligne_debut_logo 2
#define ligne_debut_remplissage_fond 25
#define hauteur_motif_fond_fullscreen_TCB 40
#define largeur_motif_fond_fullscreen_TCB  8

#define hauteur_logo_fullscreen_TCB 18
#define largeur_logo_fullscreen_TCB  264

#define nombre_sprites_fullscreen_TCB 9
#define largeur_sprite_fullscreen_TCB 17
#define hauteur_sprite_fullscreen_TCB 10

#define nombre_scrollers_fullscreen_TCB 8
#define nombre_lignes_par_lettre_scroller_fullscreen_TCB 40
#define decalage_scrollings_par_vlb_fullscreen_TCB 8
#define nb_pixels_a_faire_entrer_par_vbl_vlb_fullscreen_TCB 2

// en ns
#define TICKS_PER_FRAME 16000000

struct	animation
{
	float	angle_x_initial;
	float	angle_y_initial;
	float	angle_z_initial;
	float	increment_angle_x;
	float	increment_angle_y;
	float	increment_angle_z;
	int		duree_animation;
	float	posx_initial;
	float	posy_initial;
	float	posz_initial;

};

struct	animation liste_animation[]={
	
	// mega scroll / sprites TCB : opposés
	// mettre une attente sur fullscreen tcb
	
	// face avant = TechTech, on attend que ca démarre : 60*25
	{ .angle_x_initial=0.0f, .angle_y_initial=(MONPI*1.5f), .angle_z_initial =(-MONPI/2.0f), .increment_angle_x=0.0f, .increment_angle_y=0.0f, .increment_angle_z=0.0f, .duree_animation=(60*25), .posx_initial = 0.0f, .posy_initial = 0.0f, .posz_initial = -2.0f, },
	// on tourne vers le fullscreen TCB
	{ .angle_x_initial=0.0f, .angle_y_initial=(MONPI*1.5f), .angle_z_initial =(-MONPI/2.0f), .increment_angle_x=0.0f, .increment_angle_y=(-MONPI/2.0f)/(60*4), .increment_angle_z=0.0f, .duree_animation=60*4, .posx_initial = 0.0f, .posy_initial = 0.0f, .posz_initial = -2.0f, },
	// 1/2 seconde sur le fullscreen TCB
	{ .angle_x_initial=-1, .angle_y_initial=-1, .angle_z_initial =-1, .increment_angle_x=0.0f, .increment_angle_y=0.0f, .increment_angle_z=0.0f, .duree_animation=30, .posx_initial = 0.0f, .posy_initial = 0.0f, .posz_initial = -2.0f, },
	// on le redresse
	{ .angle_x_initial=-1, .angle_y_initial=-1, .angle_z_initial =-1, .increment_angle_x=0.0f, .increment_angle_y=0.0f, .increment_angle_z=((-MONPI/2.0f)*2)/(60*12), .duree_animation=60*12, .posx_initial = 0.0f, .posy_initial = 0.0f, .posz_initial = -2.0f, },
	// synchro musique
	{ .angle_x_initial=-1, .angle_y_initial=-1, .angle_z_initial =-1, .increment_angle_x=0.0f, .increment_angle_y=0.0f, .increment_angle_z=0.0f, .duree_animation=(60*8)+10, .posx_initial = 0.0f, .posy_initial = 0.0f, .posz_initial = -2.0f, },
	// on tourne sur le meme angle_x_encours
	{ .angle_x_initial=-1, .angle_y_initial=-1, .angle_z_initial =-1, .increment_angle_x=0.0f, .increment_angle_y=((-MONPI/2.0f)*2)/(60*3), .increment_angle_z=0.0f, .duree_animation=(60*16)-40, .posx_initial = 0.0f, .posy_initial = 0.0f, .posz_initial = -2.0f, },
	
	
	
		
	{ .angle_x_initial=-1.0f, .angle_y_initial=-1.0f, .angle_z_initial =-1.0f, .increment_angle_x=0.005f, .increment_angle_y=-0.008f, .increment_angle_z=0.006f, .duree_animation=60*30, .posx_initial = 0.0f, .posy_initial = 0.0f, .posz_initial = -2.0f, }
	
};

// variables animations
float increment_pos_rotation_x, increment_pos_rotation_y ,increment_pos_rotation_z;
float pos_x_cube_opengl , pos_y_cube_opengl  , pos_z_cube_opengl ;
int duree_animation_encours=0;
int numero_animation_en_cours=0 , nombre_total_animation=1;


struct	sprite_fullscreen_TCB
{
	//u32* pointeur_destination_precedents[nb_ecrans_fullscreen_TCB];
	u32*	pointeur_destination_precedents;
	// u32* buffers_cls[nb_ecrans_fullscreen_TCB];
	u32* buffers_cls;
	u32* dessin_sprite;
	int x_en_cours;
	int y_en_cours;

};

struct sprite_fullscreen_TCB sprites[nombre_sprites_fullscreen_TCB];


struct scroller_fullscreen_TCB
{
	int		delai_avant_demarrage;
	char*	pointeur_en_cours_dans_le_message;
	char*	pointeur_vers_debut_message;
	int		position_dans_le_caractere_actuel;
	int		largeur_caractere_actuel;
	u32*	pointeur_memoire_sur_font;
};


struct table_lettres_scrollers_fullscreen_TCB
{
	char	lettre;
	int		x;
	int		y;
	int		largeur;
};

// tables des scrollings
struct table_lettres_scrollers_fullscreen_TCB letters_fullscreen_TCB[] = 
{
	{.lettre=' ', .x=  0,   .y=0, .largeur=32},
	{.lettre='\'', .x=294,   .y=0, .largeur=16},
	{.lettre=',', .x= 84,  .y=40, .largeur=16},
	{.lettre='.', .x=168,  .y=40, .largeur=16},
	{.lettre='A', .x=126, .y=120, .largeur=42},
	{.lettre='B', .x=168, .y=120, .largeur=32},
	{.lettre='C', .x=210, .y=120, .largeur=32},
	{.lettre='D', .x=252, .y=120, .largeur=32},
	{.lettre='E', .x=294, .y=120, .largeur=32},
	{.lettre='F', .x=336, .y=120, .largeur=32},
	{.lettre='G', .x=378, .y=120, .largeur=32},
	{.lettre='H', .x=  0, .y=160, .largeur=32},
	{.lettre='I', .x= 42, .y=160, .largeur=16},
	{.lettre='J', .x= 84, .y=160, .largeur=32},
	{.lettre='K', .x=126, .y=160, .largeur=32},
	{.lettre='L', .x=168, .y=160, .largeur=32},
	{.lettre='M', .x=210, .y=160, .largeur=42},
	{.lettre='N', .x=252, .y=160, .largeur=32},
	{.lettre='O', .x=294, .y=160, .largeur=42},
	{.lettre='P', .x=336, .y=160, .largeur=32},
	{.lettre='Q', .x=378, .y=160, .largeur=42},
	{.lettre='R', .x=  0, .y=200, .largeur=32},
	{.lettre='S', .x= 42, .y=200, .largeur=32},
	{.lettre='T', .x= 84, .y=200, .largeur=32},
	{.lettre='U', .x=126, .y=200, .largeur=32},
	{.lettre='V', .x=168, .y=200, .largeur=42},
	{.lettre='W', .x=210, .y=200, .largeur=42},
	{.lettre='X', .x=252, .y=200, .largeur=32},
	{.lettre='Y', .x=294, .y=200, .largeur=32},
	{.lettre='Z', .x=336, .y=200, .largeur=32}
};


char* texte0="DONT YOU THINK THIS BEATS THE LEVEL SIXTEEN FULLSCREEN SCREEN. WE THINK SO. WE HAVE SEEN SOME PRETTY GOOD FULLSCREENS BY OMEGA AND SYNC.   WE\'RE REALLY LOOKING FORWARD TO SEEING YOUR DEMOS.   YESTERDAY TFE OF OMEGA TOLD US THAT THEIR DEMO PROBABLY WON\'T BE AS BIG AS THEY HAD HOPED. FOR INSTANCE, TFE HAD MADE SOME SERIOUS MISCALCULATIONS WHEN HE DESIGNED THEIR MAIN MENU, SO AFTER HAVING RECALCULATING, HE REALIZED THAT IT WON\'T WORK ON HALF MEG\' COMPUTERS. BAD LUCK GUYS..    LET\'S WRAZZZZZZ. e";
char* texte1="BLIIIRP BLUUURP BLIIIRP BLUUURP BLIIIRP BLUUURP BLIIIRP BLUUURP BLAAARP    LET 'S WRAP.....   SICKEST SO FAR.      e";
char* texte2="WOW, HERE COMES THE FULLSCREEN SCREEN BY THE MEGAMIGHTY CAREBEARS. NOW FOR THE CREDITS, THEY ARE SURE ARE FUN READING    ALL CODING AND SAMPLING BY THE CAREBEARS.  FONT BY TRISTAR, TCB LOGO BY TCB. OKAY, THATS ALL FOR THIS SCROLLINE, NOW YOU HAVE SEVEN OTHER SCROLLTEXTS TO READ, AND WE HAVE SEVEN MORE TO WRITE   BLEUURGH...    e";
char* texte3="NAAAAAAAAAAAAAAAAAAAAAAAAAAAAAH BAAAAAAAAAAAAAAAAAAAAAAAAAH   e";
char* texte4="WE DOUBT THAT ANYBODY WILL EVER PARALLAX SCROLL A LARGER FOUR BITPLANE AREA, WELL WE EVEN DOUBT THAT ANYONE WILL EVER SCROLL A TWO BITPLANE FIELD OF THIS SIZE, NEITHER DO WE THINK THAT ANYBODY WILL MANAGE IT WITHOUT SAMPLING OR SPRITES.  LETS WARP....    e";
char* texte5="WE WANT DONATIONS...  SEND US ALL YOUR MONEY, OR AT LEAST HALF OF IT.    END OF MESSAGE    e";
char* texte6 = "AN COOL HATES THIS SAMPLING, BUT ON THE OTHER HAND HE HATES EVERYTHING THAT HE HASN\'T MADE HIMSELF, INCLUDING THIS SCROLLTEXT.    e";
char* texte7 = "THIS IS THE LAST SCROLLTEXT WE WRITE, SO WE\'RE TOTALLY OUT OF IDEAS WHAT TO WRITE, SO LET\'S WRAP.          e";






// back[],  back_size
#include "back.h"

// font_png[], font_png_end[], font_png_size
#include "font_png.h"

// sprite_png[], sprite_size
#include "sprite_png.h"

// logo
// title_png[], title_png_end[], title_png_size
#include "title_png.h"

stbi_uc* img;
stbi_uc* fontsimg;

u32* back_rgba;

u32* pointeur_buffer_ecrans_scrolling;

u32* fonts;

int iteration;
int pos_scrolling_horizontal_dans_ecran;

struct	scroller_fullscreen_TCB	scrollers_fullscreen_TCB[nombre_scrollers_fullscreen_TCB]={};

u32* buffer_logo;



//-----------------------------------------------------------------------------
//                sprites TCB 
//-----------------------------------------------------------------------------

#define ballrec 320
#define largeur_sprites_TCB 320
#define hauteur_sprites_TCB 200
#define PILEN 720

#define RESX 320
#define RESY 192

#define RX1 76
#define RX2 76
#define RY1 44
#define RY2 44


#include "ball2_png.h"
// 16 x 16
// ball2_png
#define largeur_ball_sprites_TCB 16
#define hauteur_ball_sprites_TCB 16

#include "bg2_png.h"
// 320x200
// bg2_png

stbi_uc* fond_png ;
stbi_uc* stbiball_png;

u32* buffer_ecran;
u32* fond_ecran;

u32* ball;

float cosTab[PILEN];
float sinTab[PILEN];


// megascroll ULM
#define largeur_megascroll_ULM 384
// 697 = 17 * 41
#define hauteur_megascroll_ULM 264
// 264 / 8  = 33

stbi_uc* tile_megascroll_ULM_png;
stbi_uc* ball_megascroll_ULM_png;
stbi_uc* font_megascroll_ULM_png;

u32* buffer_ecran_megascroll_ULM;
u32* buffer_ecran__avec_marges_megascroll_ULM;

u32* pointeurtileu32;
u32* ligne_tile_megascroll_ULM;

#include "tile88_png.h"
// 8 x 8
// tile_png
#define largeur_tile_megascroll_ULM 8
#define hauteur_tile_megascroll_ULM 8

int decalage_en_cours_horizontal_megascroll_ULM=0;

#include "courbe_horizontale_ULM2w.h"
// courbe_horizontale_ULM2w[]
// courbe_horizontale_ULM2w_size

#include "table_2_valeurs_w.h"
// table_2_valeurs_w
// table_2_valeurs_w_size
int pointeur_pos_table_2_valeurs_w = 0;

float radian_mouvement_fond_vertical_megascroller_ULM=0;
float radian_mouvement_bigscroll_megascroller_ULM=0;

// la boule
#include "ball1616_png.h"
u32*	pointeur_ball_u32_megascroller_ULM;


// big scroll 
//
// fonts de 16x16
// 16 *16 = 256
// sinus de 3*16 en haut invisible a 3*16 en bas invisible mais avec limite en bas 

#include "fontULM_png.h"
// font_png
// font_png_size
// 256*64 = 16 * 4
#define longeur_ligne_font_megascroll_ULM 256
#define largeur_1font_megascroll_ULM 16
#define hauteur_1font_megascroll_ULM 16

// lire le texte scroll_text_megascroll_ULM
// si caractere = $ c est la fin
// lettre - 32 = pos dans fonts
int pos_text_scroller_megascroll_ULM = 0;

// entre 0 et 15
int pos_dans_font_scroller_megascroll_ULM = 0;

u32*	pointeur_fnt_u32_megascroller_ULM;

// a chaque frame il ajouter 16 pixels
// 24 boules par ligne
// lire la lettre
// afficher des colonnes de boules
// 		parcourir la font par colonnes

char* scroll_text_megascroll_ULM = "      OH BOY, YOU ARE SO WRONG!!! THIS IS NOT TCB BUT THE FATE OF ULM MOVING WHOLE SCREEN AND PLAYING A 15KHZ DIGISOUND...  (OHHH, AREN'T THE NUMBERS UGLY???) WE ARE NOW AT THALION IN GUETERSLOH AND IT'S 1 O'CLOCK AND THIS IS THE 2ND TIME I WRITE THIS TEXT BECAUSE I UNFORTUNATELY DESTROYED THE FIRST ONE...    I'M VERY TIRED BECAUSE WE SPEND ABOUT 5 HOURS IN GUNSTICK'S CAR AND THIS WASN'T VERY AMUSING...    THIS MORNING AT 9 O'CLOCK WE WILL HAVE TO GET UP AS TODAY THE ATARI FAIR IN DUESSELDORF STARTS...   THIS IS VERY BAD, AS I DON'T THINK THAT I CAN GO TO BED VERY SOON BECAUSE WE STILL HAVE ANOTHER SCREEN TO FINISH AND IN THIS ONE THERE ARE STILL SOME BUGS... OK, NOW I'LL DO SOME GREETINGS: MEGAGREETINGS TO ALL PEOPLE WHO ARE HERE, TO THE UNION, THE LOST BOYS, THE GARLIC EATERS, NEW POWER GENERATION, FOXX AND NEXT AND TO ALL THE OTHERS I FORGOT... I THINK I ALSO HAVE TO MENTION THE FIRE CRACKERS AND THE AEMOROIDS   I SEBD MEGAFUCKING GREETINGS TO THEM, AS THE FIRST ONES HAVE SPREAD SOME OF OUR SOURCES AND THE SECOND ONES USED THE FOR THE INTRO IN THEIR HEMOSOUND DEMO II... IT WAS A VERY OLD VERSION OF OUR INTRO FOR THE NEW YEAR DEMO WHICH DOESN'T EVEN WORK ON ALL ST'S, WHICH DOESN'T WORK ON NEARBY NO ST...   WELL, NOW I HAVE TO STOP AS I HAVE TO FINISH THE OTHER SCREEN...   BYE AND SEE YOU IN OUR MEGA DEMO!!!    $";


//-----------------------------------------------------------------------------
//               Shadow of the Beast 
//-----------------------------------------------------------------------------


#define largeur_Shadow 416
#define hauteur_Shadow 270
u32*	buffer_ecran_Shadow;

int iteration_Shadow = 0;

#include "overlayshadow_png.h"
// overlayshadow_png
#define largeur_overlay_Shadow 416
#define hauteur_overlay_Shadow 660
stbi_uc* 	pointeur_overlay_Shadow;
u32* 		pointeur_overlay_Shadow_u32;

#include "tableoverlay.h"
u32* 		pointeur_table_overlay_Shadow_u32;
int pos_table_overlay_Shadow=0;

#include "backgroundshadow_png.h"
// backgroundshadow_png
stbi_uc* 	pointeur_background_Shadow;
u32* 		pointeur_background_Shadow_u32;

struct structparallax
{
		int posx;
		int posy;
		int hauteur;
		int increment;
		int posx_actuelle;
};

// Parallax values
#define nombre_parallax_Shadow 17
#define largeur_background_Shadow 320
struct structparallax parallax []= {
                {.posx= 0, .posy=  10, .hauteur= 18, .increment= 12, .posx_actuelle=0},
                {.posx= 0, .posy=  28, .hauteur= 40, .increment= 10, .posx_actuelle=0},
                {.posx= 0, .posy=  68, .hauteur= 20, .increment=  8, .posx_actuelle=0},
                {.posx= 0, .posy=  88, .hauteur=  8, .increment=  6, .posx_actuelle=0},
                {.posx= 0, .posy=  96, .hauteur=  8, .increment=  3, .posx_actuelle=0},
                {.posx= 0, .posy= 104, .hauteur= 68, .increment=  1, .posx_actuelle=0},
                {.posx= 0, .posy= 172, .hauteur=  2, .increment=  2, .posx_actuelle=0},
                {.posx= 0, .posy= 174, .hauteur=  3, .increment=  3, .posx_actuelle=0},
                {.posx= 0, .posy= 177, .hauteur=  4, .increment=  4, .posx_actuelle=0},
                {.posx= 0, .posy= 181, .hauteur=  5, .increment=  5, .posx_actuelle=0},
                {.posx= 0, .posy= 186, .hauteur=  6, .increment=  6, .posx_actuelle=0},
                {.posx= 0, .posy= 192, .hauteur=  7, .increment=  7, .posx_actuelle=0},
                {.posx= 0, .posy= 199, .hauteur=  8, .increment=  8, .posx_actuelle=0},
                {.posx= 0, .posy= 207, .hauteur=  9, .increment=  9, .posx_actuelle=0},
                {.posx= 0, .posy= 216, .hauteur= 10, .increment= 10, .posx_actuelle=0},
                {.posx= 0, .posy= 226, .hauteur= 11, .increment= 11, .posx_actuelle=0},
                {.posx= 0, .posy= 237, .hauteur= 12, .increment= 12, .posx_actuelle=0} };


// variables sprites
#include "spritesshadow_png.h"
// spritesshadow_png
stbi_uc* 	pointeur_sprites_Shadow;
u32* 		pointeur_sprites_Shadow_u32;
#define largeur_sprites_Shadow 32
#define hauteur_sprites_Shadow 9

#include "SotB_Rip_Amiga_1_png.h"
#define largeur_png_sprites_Amiga_Shadow 440
stbi_uc* 	pointeur_Amiga_Shadow;
u32* 		pointeur_Amiga_Shadow_u32;
// arbr en 0,378 jusqu'a 134,517

#define bord_gauche_sprites_Amiga_Shadow 48
#define bord_droit_sprites_Amiga_Shadow 368

int pos_x_dest_sprite_puit_Amiga_Shadow = 240;
#define largeur_sprite_puit_Amiga_Shadow 89
#define hauteur_sprite_puit_Amiga_Shadow 83
#define pos_x_source_sprite_puit_Amiga_Shadow 149
#define pos_y_source_sprite_puit_Amiga_Shadow 434
#define pos_y_dest_sprite_puit_Amiga_Shadow 147
int flag_afficher_puit_Amiga_shadow=0;

int pos_x_dest_sprite_arbre_droite_Amiga_Shadow = 240;
#define largeur_sprite_arbre_droite_Amiga_Shadow 179
#define hauteur_sprite_arbre_droite_Amiga_Shadow 169
#define pos_y_source_sprite_arbre_droite_Amiga_Shadow 356
#define pos_y_dest_sprite_arbre_droite_Amiga_Shadow 40
#define pos_x_source_sprite_arbre_droite_Amiga_Shadow 252
int flag_afficher_arbre_droite_Amiga_shadow=0;


int pos_x_dest_sprite_arbre_gauche_Amiga_Shadow = 240;
#define largeur_sprite_arbre_gauche_Amiga_Shadow 134
#define hauteur_sprite_arbre_gauche_Amiga_Shadow 139
#define pos_y_source_sprite_arbre_gauche_Amiga_Shadow 378
#define pos_y_dest_sprite_arbre_gauche_Amiga_Shadow 70
#define pos_x_source_sprite_arbre_gauche_Amiga_Shadow 0
int flag_afficher_arbre_gauche_Amiga_shadow=0;


//-----------------------------------------------------------------------------
//               Generic
//-----------------------------------------------------------------------------

YMMUSIC* pMusic_Intro;
YMMUSIC* pMusic_Intro2;
unsigned char* pointeur_module_YM_Intro; 
unsigned char* pointeur_module_YM_Intro2;
int taille_module_YM_Intro;
int taille_module_YM_Intro2;


//#define ENABLE_NXLINK 1

// en ns
#define TICKS_PER_FRAME 16000000

//-----------------------------------------------------------------------------
// nxlink support
//-----------------------------------------------------------------------------

#ifndef ENABLE_NXLINK
#define TRACE(fmt,...) ((void)0)
#else
#include <unistd.h>
#define TRACE(fmt,...) printf("%s: " fmt "\n", __PRETTY_FUNCTION__, ## __VA_ARGS__)

static int s_nxlinkSock = -1;

static void initNxLink()
{
    if (R_FAILED(socketInitializeDefault()))
        return;

    s_nxlinkSock = nxlinkStdio();
    if (s_nxlinkSock >= 0)
        TRACE("printf output now goes to nxlink server");
    else
        socketExit();
}

static void deinitNxLink()
{
    if (s_nxlinkSock >= 0)
    {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}

extern "C" void userAppInit()
{
    initNxLink();
}

extern "C" void userAppExit()
{
    deinitNxLink();
}

#endif

// nb_ecran actuel
#define nb_ecran_actuel 4

// taille du buffer de texture
#define buffer_texture_largeur 720
#define buffer_texture_hauteur 720*8
#define buffer_texture_hateur_un_ecran 720

// taille fenetre pour cette démo
#define boink_screen_largeur 720
#define boink_screen_hauteur 720




// u32* boink_screen_buffer;
u32* texture_screen_buffer;

double pourcentageeNS=0 , pourcentageapresloadtexture=0, pourcentageapressceneupdate=0 ,pourcentageapresCLSrender=0;
double pourcentageavantdraw = 0, pourcentageapresdraw = 0;
double pourcentageeavopengl=0;
double pourcentageCalc=0,  pourcentageCLS=0,pourcentageavantCLS=0, pourcentagefinal=0;;


int	ousuisje=0;

float	tableau_sin[3600];
float	tableau_cos[3600];

float	valeur_A,valeur_B,valeur_C,valeur_D,valeur_E,valeur_F,valeur_G,valeur_H,valeur_I;
float	valeur_Q;

float cos_x,sin_x,cos_y,sin_y,cos_z,sin_z;

const u32 ca=0xfe0000ff;
const u32 cb=0xffffffff;
const u32 sh=0x00000000;

int     Zdep=0,Xdep=0,Ydep=0;
int posx2D = 0, posy2D=0;

int	incanglex=0;
int	incangley=0;
int	incanglez=0;
int	xag,yag,xbg,ybg;
int	y_min,y_max;

int	tableau_x_min[720]={0};
int	tableau_x_max[1280]={0};

u32 stride;    
u32* framebuf;

struct	struct_point
{
	float	x;
	float	y;
	float	z;
	float	xrotated;
	float	yrotated;
	float	zrotated;
	float	valeur_Q;
	int	x2D;
	int	y2D;
};

struct	struct_triangle
{
	int 	p1;
	int		p2;
	int		p3;
	//int	index_point1_visibilite;
	//int     index_point2_visibilite;
	// int     index_point3_visibilite;
	//int	nb_cotes;
	u32	color;
	int	Z_moyen;
	// liste des points formant la face
	int	tableau_cotes[20];
};







struct	face_triee
{
	int		numero_face;
	int		Z_moyen_face;
};

struct face_triee faces_triees[300];

// 288 faces
struct	struct_triangle faces_boink[]={
	{.p1=21,.p2=20,.p3=2, .color=ca},
	{.p1=22,.p2=21,.p3=3, .color=cb},
	{.p1=23,.p2=22,.p3=4, .color=ca},
	{.p1=24,.p2=23,.p3=5, .color=cb},
	{.p1=25,.p2=24,.p3=6, .color=ca},
	{.p1=26,.p2=25,.p3=7, .color=cb},
	{.p1=27,.p2=26,.p3=8, .color=ca},
	{.p1=28,.p2=27,.p3=9, .color=cb},
	{.p1=29,.p2=28,.p3=10, .color=ca},
	{.p1=30,.p2=29,.p3=11, .color=cb},
	{.p1=31,.p2=30,.p3=12, .color=ca},
	{.p1=32,.p2=31,.p3=13, .color=cb},
	{.p1=33,.p2=32,.p3=14, .color=ca},
	{.p1=34,.p2=33,.p3=15, .color=cb},
	{.p1=35,.p2=34,.p3=16, .color=ca},
	{.p1=36,.p2=35,.p3=17, .color=cb},
	{.p1=37,.p2=36,.p3=18, .color=ca},
	{.p1=20,.p2=37,.p3=19, .color=cb},
	{.p1=39,.p2=38,.p3=20, .color=cb}, 
	{.p1=40,.p2=39,.p3=21, .color=ca},
	{.p1=41,.p2=40,.p3=22, .color=cb},
	{.p1=42,.p2=41,.p3=23, .color=ca},
	{.p1=43,.p2=42,.p3=24, .color=cb},
	{.p1=44,.p2=43,.p3=25, .color=ca},
	{.p1=45,.p2=44,.p3=26, .color=cb},
	{.p1=46,.p2=45,.p3=27, .color=ca},
	{.p1=47,.p2=46,.p3=28, .color=cb},
	{.p1=48,.p2=47,.p3=29, .color=ca},
	{.p1=49,.p2=48,.p3=30, .color=cb},
	{.p1=50,.p2=49,.p3=31, .color=ca},
	{.p1=51,.p2=50,.p3=32, .color=cb},
	{.p1=52,.p2=51,.p3=33, .color=ca},
	{.p1=53,.p2=52,.p3=34, .color=cb},
	{.p1=54,.p2=53,.p3=35, .color=ca},
	{.p1=55,.p2=54,.p3=36, .color=cb}, 
	{.p1=38,.p2=55,.p3=37, .color=ca},
	{.p1=57,.p2=56,.p3=38, .color=ca},
	{.p1=58,.p2=57,.p3=39, .color=cb},
	{.p1=59,.p2=58,.p3=40, .color=ca},
	{.p1=60,.p2=59,.p3=41, .color=cb},
	{.p1=61,.p2=60,.p3=42, .color=ca},
	{.p1=62,.p2=61,.p3=43, .color=cb},
	{.p1=63,.p2=62,.p3=44, .color=ca},
	{.p1=64,.p2=63,.p3=45, .color=cb},
	{.p1=65,.p2=64,.p3=46, .color=ca},
	{.p1=66,.p2=65,.p3=47, .color=cb},
	{.p1=67,.p2=66,.p3=48, .color=ca},
	{.p1=68,.p2=67,.p3=49, .color=cb},
	{.p1=69,.p2=68,.p3=50, .color=ca},
	{.p1=70,.p2=69,.p3=51, .color=cb},
	{.p1=71,.p2=70,.p3=52, .color=ca},
	{.p1=72,.p2=71,.p3=53, .color=cb}, 
	{.p1=73,.p2=72,.p3=54, .color=ca},
	{.p1=56,.p2=73,.p3=55, .color=cb}, 
	{.p1=75,.p2=74,.p3=56, .color=cb},
	{.p1=76,.p2=75,.p3=57, .color=ca},
	{.p1=77,.p2=76,.p3=58, .color=cb},
	{.p1=78,.p2=77,.p3=59, .color=ca},
	{.p1=79,.p2=78,.p3=60, .color=cb},
	{.p1=80,.p2=79,.p3=61, .color=ca},
	{.p1=81,.p2=80,.p3=62, .color=cb},
	{.p1=82,.p2=81,.p3=63, .color=ca},
	{.p1=83,.p2=82,.p3=64, .color=cb},
	{.p1=84,.p2=83,.p3=65, .color=ca},
	{.p1=85,.p2=84,.p3=66, .color=cb},
	{.p1=86,.p2=85,.p3=67, .color=ca},
	{.p1=87,.p2=86,.p3=68, .color=cb},
	{.p1=88,.p2=87,.p3=69, .color=ca},
	{.p1=89,.p2=88,.p3=70, .color=cb},
	{.p1=90,.p2=89,.p3=71, .color=ca},
	{.p1=91,.p2=90,.p3=72, .color=cb}, 
	{.p1=74,.p2=91,.p3=73, .color=ca},
	{.p1=93,.p2=92,.p3=74, .color=ca},
	{.p1=94,.p2=93,.p3=75, .color=cb},
	{.p1=95,.p2=94,.p3=76, .color=ca},
	{.p1=96,.p2=95,.p3=77, .color=cb},
	{.p1=97,.p2=96,.p3=78, .color=ca},
	{.p1=98,.p2=97,.p3=79, .color=cb},
	{.p1=99,.p2=98,.p3=80, .color=ca},
	{.p1=100,.p2=99,.p3=81, .color=cb},
	{.p1=101,.p2=100,.p3=82, .color=ca},
	{.p1=102,.p2=101,.p3=83, .color=cb},
	{.p1=103,.p2=102,.p3=84, .color=ca},
	{.p1=104,.p2=103,.p3=85, .color=cb},
	{.p1=105,.p2=104,.p3=86, .color=ca},
	{.p1=106,.p2=105,.p3=87, .color=cb},
	{.p1=107,.p2=106,.p3=88, .color=ca},
	{.p1=108,.p2=107,.p3=89, .color=cb},
	{.p1=109,.p2=108,.p3=90, .color=ca},
	{.p1=92,.p2=109,.p3=91, .color=cb}, 
	{.p1=111,.p2=110,.p3=92, .color=cb},
	{.p1=112,.p2=111,.p3=93, .color=ca},
	{.p1=113,.p2=112,.p3=94, .color=cb},
	{.p1=114,.p2=113,.p3=95, .color=ca},
	{.p1=115,.p2=114,.p3=96, .color=cb},
	{.p1=116,.p2=115,.p3=97, .color=ca},
	{.p1=117,.p2=116,.p3=98, .color=cb},
	{.p1=118,.p2=117,.p3=99, .color=ca},
	{.p1=119,.p2=118,.p3=100, .color=cb},
	{.p1=120,.p2=119,.p3=101, .color=ca},
	{.p1=121,.p2=120,.p3=102, .color=cb},
	{.p1=122,.p2=121,.p3=103, .color=ca},
	{.p1=123,.p2=122,.p3=104, .color=cb},
	{.p1=124,.p2=123,.p3=105, .color=ca},
	{.p1=125,.p2=124,.p3=106, .color=cb},
	{.p1=126,.p2=125,.p3=107, .color=ca},
	{.p1=127,.p2=126,.p3=108, .color=cb}, 
	{.p1=110,.p2=127,.p3=109, .color=ca},
	{.p1=129,.p2=128,.p3=110, .color=ca},
	{.p1=130,.p2=129,.p3=111, .color=cb},
	{.p1=131,.p2=130,.p3=112, .color=ca},
	{.p1=132,.p2=131,.p3=113, .color=cb},
	{.p1=133,.p2=132,.p3=114, .color=ca},
	{.p1=134,.p2=133,.p3=115, .color=cb},
	{.p1=135,.p2=134,.p3=116, .color=ca},
	{.p1=136,.p2=135,.p3=117, .color=cb},
	{.p1=137,.p2=136,.p3=118, .color=ca},
	{.p1=138,.p2=137,.p3=119, .color=cb},
	{.p1=139,.p2=138,.p3=120, .color=ca},
	{.p1=140,.p2=139,.p3=121, .color=cb},
	{.p1=141,.p2=140,.p3=122, .color=ca},
	{.p1=142,.p2=141,.p3=123, .color=cb},
	{.p1=143,.p2=142,.p3=124, .color=ca},
	{.p1=144,.p2=143,.p3=125, .color=cb},
	{.p1=145,.p2=144,.p3=126, .color=ca},
	{.p1=128,.p2=145,.p3=127, .color=cb}, 
	{.p1=3,.p2=2,.p3=0, .color=cb},
	{.p1=1,.p2=128,.p3=129, .color=cb},
	{.p1=4,.p2=3,.p3=0, .color=ca},
	{.p1=1,.p2=129,.p3=130, .color=ca},
	{.p1=5,.p2=4,.p3=0, .color=cb},
	{.p1=1,.p2=130,.p3=131, .color=cb},
	{.p1=6,.p2=5,.p3=0, .color=ca},
	{.p1=1,.p2=131,.p3=132, .color=ca},
	{.p1=7,.p2=6,.p3=0, .color=cb},
	{.p1=1,.p2=132,.p3=133, .color=cb},
	{.p1=8,.p2=7,.p3=0, .color=ca},
	{.p1=1,.p2=133,.p3=134, .color=ca},
	{.p1=9,.p2=8,.p3=0, .color=cb},
	{.p1=1,.p2=134,.p3=135, .color=cb},
	{.p1=10,.p2=9,.p3=0, .color=ca},
	{.p1=1,.p2=135,.p3=136, .color=ca},
	{.p1=11,.p2=10,.p3=0, .color=cb},
	{.p1=1,.p2=136,.p3=137, .color=cb},
	{.p1=12,.p2=11,.p3=0, .color=ca},
	{.p1=1,.p2=137,.p3=138, .color=ca},
	{.p1=13,.p2=12,.p3=0, .color=cb},
	{.p1=1,.p2=138,.p3=139, .color=cb},
	{.p1=14,.p2=13,.p3=0, .color=ca},
	{.p1=1,.p2=139,.p3=140, .color=ca},
	{.p1=15,.p2=14,.p3=0, .color=cb},
	{.p1=1,.p2=140,.p3=141, .color=cb},
	{.p1=16,.p2=15,.p3=0, .color=ca},
	{.p1=1,.p2=141,.p3=142, .color=ca},
	{.p1=17,.p2=16,.p3=0, .color=cb},
	{.p1=1,.p2=142,.p3=143, .color=cb},
	{.p1=18,.p2=17,.p3=0, .color=ca},
	{.p1=1,.p2=143,.p3=144, .color=ca},
	{.p1=19,.p2=18,.p3=0, .color=cb},
	{.p1=1,.p2=144,.p3=145, .color=cb},
	{.p1=2,.p2=19,.p3=0, .color=ca},
	{.p1=1,.p2=145,.p3=128, .color=ca},
	{.p1=3,.p2=21,.p3=2, .color=ca},
	{.p1=4,.p2=22,.p3=3, .color=cb}, 
	{.p1=5,.p2=23,.p3=4, .color=ca},
	{.p1=6,.p2=24,.p3=5, .color=cb},
	{.p1=7,.p2=25,.p3=6, .color=ca},
	{.p1=8,.p2=26,.p3=7, .color=cb},
	{.p1=9,.p2=27,.p3=8, .color=ca},
	{.p1=10,.p2=28,.p3=9, .color=cb},
	{.p1=11,.p2=29,.p3=10, .color=ca},
	{.p1=12,.p2=30,.p3=11, .color=cb},
	{.p1=13,.p2=31,.p3=12, .color=ca},
	{.p1=14,.p2=32,.p3=13, .color=cb},
	{.p1=15,.p2=33,.p3=14, .color=ca},
	{.p1=16,.p2=34,.p3=15, .color=cb},
	{.p1=17,.p2=35,.p3=16, .color=ca},
	{.p1=18,.p2=36,.p3=17, .color=cb},
	{.p1=19,.p2=37,.p3=18, .color=ca},
	{.p1=2,.p2=20,.p3=19, .color=cb}, 
	{.p1=21,.p2=39,.p3=20, .color=cb},
	{.p1=22,.p2=40,.p3=21, .color=ca},
	{.p1=23,.p2=41,.p3=22, .color=cb},
	{.p1=24,.p2=42,.p3=23, .color=ca},
	{.p1=25,.p2=43,.p3=24, .color=cb},
	{.p1=26,.p2=44,.p3=25, .color=ca},
	{.p1=27,.p2=45,.p3=26, .color=cb},
	{.p1=28,.p2=46,.p3=27, .color=ca},
	{.p1=29,.p2=47,.p3=28, .color=cb},
	{.p1=30,.p2=48,.p3=29, .color=ca},
	{.p1=31,.p2=49,.p3=30, .color=cb},
	{.p1=32,.p2=50,.p3=31, .color=ca},
	{.p1=33,.p2=51,.p3=32, .color=cb},
	{.p1=34,.p2=52,.p3=33, .color=ca},
	{.p1=35,.p2=53,.p3=34, .color=cb},
	{.p1=36,.p2=54,.p3=35, .color=ca},
	{.p1=37,.p2=55,.p3=36, .color=cb}, 
	{.p1=20,.p2=38,.p3=37, .color=ca},
	{.p1=39,.p2=57,.p3=38, .color=ca},
	{.p1=40,.p2=58,.p3=39, .color=cb},
	{.p1=41,.p2=59,.p3=40, .color=ca},
	{.p1=42,.p2=60,.p3=41, .color=cb},
	{.p1=43,.p2=61,.p3=42, .color=ca},
	{.p1=44,.p2=62,.p3=43, .color=cb},
	{.p1=45,.p2=63,.p3=44, .color=ca},
	{.p1=46,.p2=64,.p3=45, .color=cb},
	{.p1=47,.p2=65,.p3=46, .color=ca},
	{.p1=48,.p2=66,.p3=47, .color=cb},
	{.p1=49,.p2=67,.p3=48, .color=ca},
	{.p1=50,.p2=68,.p3=49, .color=cb},
	{.p1=51,.p2=69,.p3=50, .color=ca},
	{.p1=52,.p2=70,.p3=51, .color=cb},
	{.p1=53,.p2=71,.p3=52, .color=ca},
	{.p1=54,.p2=72,.p3=53, .color=cb},
	{.p1=55,.p2=73,.p3=54, .color=ca},
	{.p1=38,.p2=56,.p3=55, .color=cb}, 
	{.p1=57,.p2=75,.p3=56, .color=cb},
	{.p1=58,.p2=76,.p3=57, .color=ca},
	{.p1=59,.p2=77,.p3=58, .color=cb},
	{.p1=60,.p2=78,.p3=59, .color=ca},
	{.p1=61,.p2=79,.p3=60, .color=cb},
	{.p1=62,.p2=80,.p3=61, .color=ca},
	{.p1=63,.p2=81,.p3=62, .color=cb},
	{.p1=64,.p2=82,.p3=63, .color=ca},
	{.p1=65,.p2=83,.p3=64, .color=cb},
	{.p1=66,.p2=84,.p3=65, .color=ca},
	{.p1=67,.p2=85,.p3=66, .color=cb},
	{.p1=68,.p2=86,.p3=67, .color=ca},
	{.p1=69,.p2=87,.p3=68, .color=cb},
	{.p1=70,.p2=88,.p3=69, .color=ca},
	{.p1=71,.p2=89,.p3=70, .color=cb},
	{.p1=72,.p2=90,.p3=71, .color=ca},
	{.p1=73,.p2=91,.p3=72, .color=cb}, 
	{.p1=56,.p2=74,.p3=73, .color=ca},
	{.p1=75,.p2=93,.p3=74, .color=ca},
	{.p1=76,.p2=94,.p3=75, .color=cb},
	{.p1=77,.p2=95,.p3=76, .color=ca},
	{.p1=78,.p2=96,.p3=77, .color=cb},
	{.p1=79,.p2=97,.p3=78, .color=ca},
	{.p1=80,.p2=98,.p3=79, .color=cb},
	{.p1=81,.p2=99,.p3=80, .color=ca},
	{.p1=82,.p2=100,.p3=81, .color=cb},
	{.p1=83,.p2=101,.p3=82, .color=ca},
	{.p1=84,.p2=102,.p3=83, .color=cb},
	{.p1=85,.p2=103,.p3=84, .color=ca},
	{.p1=86,.p2=104,.p3=85, .color=cb},
	{.p1=87,.p2=105,.p3=86, .color=ca},
	{.p1=88,.p2=106,.p3=87, .color=cb},
	{.p1=89,.p2=107,.p3=88, .color=ca},
	{.p1=90,.p2=108,.p3=89, .color=cb},
	{.p1=91,.p2=109,.p3=90, .color=ca},
	{.p1=74,.p2=92,.p3=91, .color=cb}, 
	{.p1=93,.p2=111,.p3=92, .color=cb},
	{.p1=94,.p2=112,.p3=93, .color=ca},
	{.p1=95,.p2=113,.p3=94, .color=cb},
	{.p1=96,.p2=114,.p3=95, .color=ca},
	{.p1=97,.p2=115,.p3=96, .color=cb},
	{.p1=98,.p2=116,.p3=97, .color=ca},
	{.p1=99,.p2=117,.p3=98, .color=cb},
	{.p1=100,.p2=118,.p3=99, .color=ca},
	{.p1=101,.p2=119,.p3=100, .color=cb},
	{.p1=102,.p2=120,.p3=101, .color=ca},
	{.p1=103,.p2=121,.p3=102, .color=cb},
	{.p1=104,.p2=122,.p3=103, .color=ca},
	{.p1=105,.p2=123,.p3=104, .color=cb},
	{.p1=106,.p2=124,.p3=105, .color=ca},
	{.p1=107,.p2=125,.p3=106, .color=cb},
	{.p1=108,.p2=126,.p3=107, .color=ca},
	{.p1=109,.p2=127,.p3=108, .color=cb}, 
	{.p1=92,.p2=110,.p3=109, .color=ca},
	{.p1=111,.p2=129,.p3=110, .color=ca},
	{.p1=112,.p2=130,.p3=111, .color=cb},
	{.p1=113,.p2=131,.p3=112, .color=ca},
	{.p1=114,.p2=132,.p3=113, .color=cb},
	{.p1=115,.p2=133,.p3=114, .color=ca},
	{.p1=116,.p2=134,.p3=115, .color=cb},
	{.p1=117,.p2=135,.p3=116, .color=ca},
	{.p1=118,.p2=136,.p3=117, .color=cb},
	{.p1=119,.p2=137,.p3=118, .color=ca},
	{.p1=120,.p2=138,.p3=119, .color=cb},
	{.p1=121,.p2=139,.p3=120, .color=ca},
	{.p1=122,.p2=140,.p3=121, .color=cb},
	{.p1=123,.p2=141,.p3=122, .color=ca},
	{.p1=124,.p2=142,.p3=123, .color=cb},
	{.p1=125,.p2=143,.p3=124, .color=ca},
	{.p1=126,.p2=144,.p3=125, .color=cb},
	{.p1=127,.p2=145,.p3=126, .color=ca},
	{.p1=110,.p2=128,.p3=127, .color=cb} 
};

struct  struct_point points[]={
	{.x=0,.y=-100,.z=0},
	{.x=0,.y=100,.z=0},
	{.x=34.202014,.y=-93.969262,.z=0},
	{.x=32.13938,.y=-93.969262,.z=-11.697778},
	{.x=26.200263,.y=-93.969262,.z=-21.984631},
	{.x=17.101007,.y=-93.969262,.z=-29.619813},
	{.x=5.939117,.y=-93.969262,.z=-33.682409},
	{.x=-5.939117,.y=-93.969262,.z=-33.682409},
	{.x=-17.101007,.y=-93.969262,.z=-29.619813},
	{.x=-26.200263,.y=-93.969262,.z=-21.984631},
	{.x=-32.13938,.y=-93.969262,.z=-11.697778},
	{.x=-34.202014,.y=-93.969262,.z=0},
	{.x=-32.13938,.y=-93.969262,.z=11.697778},
	{.x=-26.200263,.y=-93.969262,.z=21.984631},
	{.x=-17.101007,.y=-93.969262,.z=29.619813},
	{.x=-5.939117,.y=-93.969262,.z=33.682409},
	{.x=5.939117,.y=-93.969262,.z=33.682409},
	{.x=17.101007,.y=-93.969262,.z=29.619813},
	{.x=26.200263,.y=-93.969262,.z=21.984631},
	{.x=32.13938,.y=-93.969262,.z=11.697778},
	{.x=64.278761,.y=-76.604444,.z=0},
	{.x=60.402277,.y=-76.604444,.z=-21.984631},
	{.x=49.240388,.y=-76.604444,.z=-41.317591},
	{.x=32.13938,.y=-76.604444,.z=-55.66704},
	{.x=11.16189,.y=-76.604444,.z=-63.302222},
	{.x=-11.16189,.y=-76.604444,.z=-63.302222},
	{.x=-32.13938,.y=-76.604444,.z=-55.66704},
	{.x=-49.240388,.y=-76.604444,.z=-41.317591},
	{.x=-60.402277,.y=-76.604444,.z=-21.984631},
	{.x=-64.278761,.y=-76.604444,.z=0},
	{.x=-60.402277,.y=-76.604444,.z=21.984631},
	{.x=-49.240388,.y=-76.604444,.z=41.317591},
	{.x=-32.13938,.y=-76.604444,.z=55.66704},
	{.x=-11.16189,.y=-76.604444,.z=63.302222},
	{.x=11.16189,.y=-76.604444,.z=63.302222},
	{.x=32.13938,.y=-76.604444,.z=55.66704},
	{.x=49.240388,.y=-76.604444,.z=41.317591},
	{.x=60.402277,.y=-76.604444,.z=21.984631},
	{.x=86.60254,.y=-50,.z=0},
	{.x=81.379768,.y=-50,.z=-29.619813},
	{.x=66.341395,.y=-50,.z=-55.66704},
	{.x=43.30127,.y=-50,.z=-75},
	{.x=15.038373,.y=-50,.z=-85.286853},
	{.x=-15.038373,.y=-50,.z=-85.286853},
	{.x=-43.30127,.y=-50,.z=-75},
	{.x=-66.341395,.y=-50,.z=-55.66704},
	{.x=-81.379768,.y=-50,.z=-29.619813},
	{.x=-86.60254,.y=-50,.z=0},
	{.x=-81.379768,.y=-50,.z=29.619813},
	{.x=-66.341395,.y=-50,.z=55.66704},
	{.x=-43.30127,.y=-50,.z=75},
	{.x=-15.038373,.y=-50,.z=85.286853},
	{.x=15.038373,.y=-50,.z=85.286853},
	{.x=43.30127,.y=-50,.z=75},
	{.x=66.341395,.y=-50,.z=55.66704},
	{.x=81.379768,.y=-50,.z=29.619813},
	{.x=98.480775,.y=-17.364818,.z=0},
	{.x=92.541658,.y=-17.364818,.z=-33.682409},
	{.x=75.440651,.y=-17.364818,.z=-63.302222},
	{.x=49.240388,.y=-17.364818,.z=-85.286853},
	{.x=17.101007,.y=-17.364818,.z=-96.984631},
	{.x=-17.101007,.y=-17.364818,.z=-96.984631},
	{.x=-49.240388,.y=-17.364818,.z=-85.286853},
	{.x=-75.440651,.y=-17.364818,.z=-63.302222},
	{.x=-92.541658,.y=-17.364818,.z=-33.682409},
	{.x=-98.480775,.y=-17.364818,.z=0},
	{.x=-92.541658,.y=-17.364818,.z=33.682409},
	{.x=-75.440651,.y=-17.364818,.z=63.302222},
	{.x=-49.240388,.y=-17.364818,.z=85.286853},
	{.x=-17.101007,.y=-17.364818,.z=96.984631},
	{.x=17.101007,.y=-17.364818,.z=96.984631},
	{.x=49.240388,.y=-17.364818,.z=85.286853},
	{.x=75.440651,.y=-17.364818,.z=63.302222},
	{.x=92.541658,.y=-17.364818,.z=33.682409},
	{.x=98.480775,.y=17.364818,.z=0},
	{.x=92.541658,.y=17.364818,.z=-33.682409},
	{.x=75.440651,.y=17.364818,.z=-63.302222},
	{.x=49.240388,.y=17.364818,.z=-85.286853},
	{.x=17.101007,.y=17.364818,.z=-96.984631},
	{.x=-17.101007,.y=17.364818,.z=-96.984631},
	{.x=-49.240388,.y=17.364818,.z=-85.286853},
	{.x=-75.440651,.y=17.364818,.z=-63.302222},
	{.x=-92.541658,.y=17.364818,.z=-33.682409},
	{.x=-98.480775,.y=17.364818,.z=0},
	{.x=-92.541658,.y=17.364818,.z=33.682409},
	{.x=-75.440651,.y=17.364818,.z=63.302222},
	{.x=-49.240388,.y=17.364818,.z=85.286853},
	{.x=-17.101007,.y=17.364818,.z=96.984631},
	{.x=17.101007,.y=17.364818,.z=96.984631},
	{.x=49.240388,.y=17.364818,.z=85.286853},
	{.x=75.440651,.y=17.364818,.z=63.302222},
	{.x=92.541658,.y=17.364818,.z=33.682409},
	{.x=86.60254,.y=50,.z=0},
	{.x=81.379768,.y=50,.z=-29.619813},
	{.x=66.341395,.y=50,.z=-55.66704},
	{.x=43.30127,.y=50,.z=-75},
	{.x=15.038373,.y=50,.z=-85.286853},
	{.x=-15.038373,.y=50,.z=-85.286853},
	{.x=-43.30127,.y=50,.z=-75},
	{.x=-66.341395,.y=50,.z=-55.66704},
	{.x=-81.379768,.y=50,.z=-29.619813},
	{.x=-86.60254,.y=50,.z=0},
	{.x=-81.379768,.y=50,.z=29.619813},
	{.x=-66.341395,.y=50,.z=55.66704},
	{.x=-43.30127,.y=50,.z=75},
	{.x=-15.038373,.y=50,.z=85.286853},
	{.x=15.038373,.y=50,.z=85.286853},
	{.x=43.30127,.y=50,.z=75},
	{.x=66.341395,.y=50,.z=55.66704},
	{.x=81.379768,.y=50,.z=29.619813},
	{.x=64.278761,.y=76.604444,.z=0},
	{.x=60.402277,.y=76.604444,.z=-21.984631},
	{.x=49.240388,.y=76.604444,.z=-41.317591},
	{.x=32.13938,.y=76.604444,.z=-55.66704},
	{.x=11.16189,.y=76.604444,.z=-63.302222},
	{.x=-11.16189,.y=76.604444,.z=-63.302222},
	{.x=-32.13938,.y=76.604444,.z=-55.66704},
	{.x=-49.240388,.y=76.604444,.z=-41.317591},
	{.x=-60.402277,.y=76.604444,.z=-21.984631},
	{.x=-64.278761,.y=76.604444,.z=0},
	{.x=-60.402277,.y=76.604444,.z=21.984631},
	{.x=-49.240388,.y=76.604444,.z=41.317591},
	{.x=-32.13938,.y=76.604444,.z=55.66704},
	{.x=-11.16189,.y=76.604444,.z=63.302222},
	{.x=11.16189,.y=76.604444,.z=63.302222},
	{.x=32.13938,.y=76.604444,.z=55.66704},
	{.x=49.240388,.y=76.604444,.z=41.317591},
	{.x=60.402277,.y=76.604444,.z=21.984631},
	{.x=34.202014,.y=93.969262,.z=0},
	{.x=32.13938,.y=93.969262,.z=-11.697778},
	{.x=26.200263,.y=93.969262,.z=-21.984631},
	{.x=17.101007,.y=93.969262,.z=-29.619813},
	{.x=5.939117,.y=93.969262,.z=-33.682409},
	{.x=-5.939117,.y=93.969262,.z=-33.682409},
	{.x=-17.101007,.y=93.969262,.z=-29.619813},
	{.x=-26.200263,.y=93.969262,.z=-21.984631},
	{.x=-32.13938,.y=93.969262,.z=-11.697778},
	{.x=-34.202014,.y=93.969262,.z=0},
	{.x=-32.13938,.y=93.969262,.z=11.697778},
	{.x=-26.200263,.y=93.969262,.z=21.984631},
	{.x=-17.101007,.y=93.969262,.z=29.619813},
	{.x=-5.939117,.y=93.969262,.z=33.682409},
	{.x=5.939117,.y=93.969262,.z=33.682409},
	{.x=17.101007,.y=93.969262,.z=29.619813},
	{.x=26.200263,.y=93.969262,.z=21.984631},
	{.x=32.13938,.y=93.969262,.z=11.697778},
};


struct  struct_objet
{
	int	nb_points;
	struct_point*	liste_des_points;
	struct_triangle* liste_des_triangles;
    int     nb_faces;
	//struct  struct_face     tableau_faces[128];
};

int nbvbl=0;
u64 initialTick,finaltick;
u64 currentTick;
//u64 startTick;
//startTick = armGetSystemTick();
u64 previousTick = armGetSystemTick();
	float toto;
	u64 diffrenceTick;
	u64 differenceNS;


struct_objet boink;

struct  struct_objet	*objet_en_cours;


//////////////////////////////////////
//   Debut des routines
//////////////////////////////////////

///// OUTtro //////////////////////

static const char* const vertexShaderSource_outtro = R"text(

    #version 410
	precision mediump float;
    
	out vec2 textureCoords;

void main() 
	{
		float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); 
		float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); 

		gl_Position = vec4(-1.0f + x*2.0f, -1.0f+y*2.0f, 0.0f, 1.0f);
		textureCoords = vec2(x, y);
	}


)text";

static const char* const fragmentShaderSource_outtro = R"text(
    #version 410
	precision mediump float;

	const float PI = 3.1415926535897932;



	in vec2 textureCoords;
	uniform sampler2D textureouttro;
	uniform float itime;
	
	out vec4 fragColor;

	
void main (void)
{
	vec2 v_texcoord = textureCoords * vec2(1.0, -1.0);
	
	vec2 u_k = vec2(10.0 , 10.0);
	float v = 0.0;
    vec2 c = v_texcoord * u_k - u_k/2.0;
    v += sin((c.x+itime));
    v += sin((c.y+itime)/2.0);
    v += sin((c.x+c.y+itime)/2.0);
    c += u_k/2.0 * vec2(sin(itime/3.0), cos(itime/2.0));
    v += sin(sqrt(c.x*c.x+c.y*c.y+1.0)+itime);
    v = v/2.0;
    vec3 col = vec3(1, sin(PI*v), cos(PI*v));
	vec4 plasma = vec4(col*.5 + .5, 1);
	vec4 fragcoloredz = texture(textureouttro, v_texcoord);
	//fragColor = vec4(mix(fragcoloredz.xyz ,plasma.xyz , 0.5),1.0);
	
	fragColor = fragcoloredz * fragcoloredz.a + plasma * (1.0 - fragcoloredz.a);
	
	// fragColor = vec4(col*.5 + .5, 1);
	
	
    
}

)text";

static GLuint s_program_outtro;
static GLuint s_vao_outtro, s_vbo_outtro;
static GLint loc_itime;
static GLint loc_textureouttro;

static GLuint createAndCompileShader(GLenum type, const char* source)
{
    GLint success;
    GLchar msg[512];

    GLuint handle = glCreateShader(type);
    if (!handle)
    {
        TRACE("%u: cannot create shader", type);
        return 0;
    }
    glShaderSource(handle, 1, &source, nullptr);
    glCompileShader(handle);
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(handle, sizeof(msg), nullptr, msg);
        TRACE("%u: %s\n", type, msg);
        glDeleteShader(handle);
        return 0;
    }

    return handle;
}


static void sceneInit_outtro()
{

	GLint vsh_outtro = createAndCompileShader(GL_VERTEX_SHADER, vertexShaderSource_outtro);
    GLint fsh_outtro = createAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource_outtro);

    s_program_outtro = glCreateProgram();
    glAttachShader(s_program_outtro, vsh_outtro);
    glAttachShader(s_program_outtro, fsh_outtro);
    glLinkProgram(s_program_outtro);

    GLint success_outtro;
    glGetProgramiv(s_program_outtro, GL_LINK_STATUS, &success_outtro);
    if (!success_outtro)
    {
        char buf_outtro[512];
        glGetProgramInfoLog(s_program_outtro, sizeof(buf_outtro), nullptr, buf_outtro);
        TRACE("Link error: %s", buf_outtro);
    }
    glDeleteShader(vsh_outtro);
    glDeleteShader(fsh_outtro);
    
	
	static GLuint s_tex_outtro;

	loc_itime = glGetUniformLocation(s_program_outtro, "itime");
	
	loc_textureouttro = glGetUniformLocation(s_program_outtro, "textureouttro");


    glGenVertexArrays(1, &s_vao_outtro);
    glGenBuffers(1, &s_vbo_outtro);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(s_vao_outtro);

    glBindBuffer(GL_ARRAY_BUFFER, s_vbo_outtro);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);


    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    //glBindVertexArray(0);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE); 
	
	    // Textures
    glGenTextures(1, &s_tex_outtro);
    glActiveTexture(GL_TEXTURE0 + 1); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, s_tex_outtro);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	//glUniform1i(uniforms[UNIFORM_VIDEOFRAME], 0);   
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, largeur_texture_outtro, hauteur_texture_outtro, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_ecran_outtro);
	
}

static void sceneRender_outtro()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(s_program_outtro);
    glBindVertexArray(s_vao_outtro); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void sceneExit_outtro()
{
    glDeleteBuffers(1, &s_vbo_outtro);
    glDeleteVertexArrays(1, &s_vao_outtro);
    glDeleteProgram(s_program_outtro);
}



///// intro //////////////////////
// la musique en thread
void remplissage_module_intro(void *args)
{
	
	    // Make sure the sample buffer size is aligned to 0x1000 bytes.
    u32 data_size = (SAMPLE_PER_BUF * CHANNELCOUNT * BYTESPERSAMPLE) ;

    u32 buffer_size = (data_size + 0xfff) & ~0xfff;

    // Allocate the buffer.
    void* out_buf_data1 = memalign(0x1000, buffer_size);
	void* out_buf_data2 = memalign(0x1000, buffer_size);
	
	u8* buffer_audio_temp = (u8*) malloc(SAMPLE_PER_BUFYM * CHANNELCOUNTYM * BYTESPERSAMPLEYM);
	
	memset(out_buf_data1, 0, buffer_size);
	memset(out_buf_data2, 0, buffer_size);

    // Initialize the default audio output device.
    rc = audoutInitialize();

	pointeur_buffer1 = &audout_buf1;
	// Prepare the audio data source buffer.
	pointeur_buffer1->next = NULL;
	pointeur_buffer1->buffer = out_buf_data1;
	pointeur_buffer1->buffer_size = buffer_size;
	pointeur_buffer1->data_size = data_size;
	pointeur_buffer1->data_offset = 0;
	
	pointeur_buffer2 = &audout_buf2;
	// Prepare the audio data source buffer.
	pointeur_buffer2->next = NULL;
	pointeur_buffer2->buffer = out_buf_data2;
	pointeur_buffer2->buffer_size = buffer_size;
	pointeur_buffer2->data_size = data_size;
	pointeur_buffer2->data_offset = 0;
	
	// init YM Intro
	pointeur_module_YM_Intro = (unsigned char*) &seven_ym_bin;
	taille_module_YM_Intro = seven_ym_bin_size;
	pMusic_Intro = ymMusicCreate();
	ymMusicLoadMemory(pMusic_Intro, pointeur_module_YM_Intro  ,taille_module_YM_Intro );
	ymMusicSetLoopMode(pMusic_Intro,YMTRUE);
	ymMusicPlay(pMusic_Intro);

	// init YM Intro2
	pointeur_module_YM_Intro2	= (unsigned char*) &union_ym_bin;
	taille_module_YM_Intro2 = union_ym_bin_size;
	pMusic_Intro2 = ymMusicCreate();
	ymMusicLoadMemory(pMusic_Intro2, pointeur_module_YM_Intro2  ,taille_module_YM_Intro2);
	ymMusicSetLoopMode(pMusic_Intro2,YMTRUE);
	ymMusicPlay(pMusic_Intro2);

	//ymMusicCompute(pMusic_Intro, (ymsample*) pointeur_buffer1->buffer ,SAMPLE_PER_BUF);
	rc = audoutAppendAudioOutBuffer(pointeur_buffer1);
	//ymMusicCompute(pMusic_Intro, (ymsample*) pointeur_buffer2->buffer ,SAMPLE_PER_BUF);
	rc = audoutAppendAudioOutBuffer(pointeur_buffer2);

    // Start audio playback.
    rc = audoutStartAudioOut();
	
	while(runningIntro)
	{
		
		rc = audoutGetReleasedAudioOutBuffer(&audout_released_buf, &nbbufrelease);
		if (nbbufrelease == 1)
				{
					if (numero_musique==1) ymMusicCompute(pMusic_Intro, (ymsample*) buffer_audio_temp ,SAMPLE_PER_BUFYM);
					if (numero_musique==2) ymMusicCompute(pMusic_Intro2, (ymsample*) buffer_audio_temp ,SAMPLE_PER_BUFYM);
			
					// recopie basic, 16 bits par 16 bits
					u16* pointeur_source_audioYM = (u16*) buffer_audio_temp;
					u16* pointeur_dest_audioYM = (u16*) audout_released_buf->buffer;
			
					int post_dest_audioYM=0;
					int pos_source_audioYM = 0;
			
					float pos_en_cours_source_a_virgule=0;
					float increment_source = (float) ((float) SAMPLERATEYM / (float) SAMPLERATE);
					// 44100 / 48000 = inferieur a 1
			
					for (int i=0;i<SAMPLE_PER_BUF;i++)
					{
						pos_source_audioYM = (int) pos_en_cours_source_a_virgule;
						s16 sample=pointeur_source_audioYM[pos_source_audioYM];
						
						sample = sample / diviseur_volume_global;
						pos_en_cours_source_a_virgule = pos_en_cours_source_a_virgule + increment_source;
				
						// a gauche
						pointeur_dest_audioYM[post_dest_audioYM] = sample;
						post_dest_audioYM++;
						// a droite
						pointeur_dest_audioYM[post_dest_audioYM] = sample;
						post_dest_audioYM++;
					}
			
					
					rc = audoutAppendAudioOutBuffer(audout_released_buf);
				}
		svcSleepThread(7000000); 
		audrenWaitFrame();
	}
	threadExit();
}



//////////////////////////////////////
// routines TechTech
//////////////////////////////////////

// routines scrollings 8x8
// scrolling 8x8 haut

static void avance_scrolling_8x8h_TechTech()
{
	// +8
	pos_dans_fonte_scrolling_8x8h_TechTech = pos_dans_fonte_scrolling_8x8h_TechTech + 2;
	
	if (pos_dans_fonte_scrolling_8x8h_TechTech == largeur_caractere_scrolling_8x8_TechTech)
	{
		// nouveau caractere
		pos_dans_fonte_scrolling_8x8h_TechTech = 0;
		pos_dans_texte_scrolling_8x8h_TechTech++;
		int lettre = (int) text_scrolltext_8x8h_TechTech[pos_dans_texte_scrolling_8x8h_TechTech];
		lettre = lettre - 32;
		// $ ?
		if (lettre == 4)
		{	pos_dans_texte_scrolling_8x8h_TechTech=0;
			lettre = (int) text_scrolltext_8x8h_TechTech[pos_dans_texte_scrolling_8x8h_TechTech];
			lettre = lettre - 32;
		}
		pointeur_pos_caractere_en_cours_scrolling_8x8h_TechTech = pointeur_fonte_scrolling_8x8_TechTech_u32 + (( lettre / 10 ) * ( hauteur_caractere_scrolling_8x8_TechTech * largeur_caractere_scrolling_8x8_TechTech * 10)) + ((lettre % 10) * largeur_caractere_scrolling_8x8_TechTech);
	}
			
		
}

static void affiche_scrolling_8x8h_TechTech()
{
	// sur largeur_TechTech de large
	
	// le scrolling a la ligne 4
	int pos_dest = largeur_TechTech * position_verticale_scrolling_8x8h_TechTech ;
	u32	pixel;
	// de 0 a largeur_caractereraster_scrolling_bas_TechTech - pos_dans caracteres
	for (int i=pos_dans_fonte_scrolling_8x8h_TechTech;i<largeur_caractere_scrolling_8x8_TechTech;i++)
	{
		int post_dest_verticale = pos_dest;
		for (int j=0;j<hauteur_caractere_scrolling_8x8_TechTech;j++)
		{
			pixel = pointeur_pos_caractere_en_cours_scrolling_8x8h_TechTech[(j*largeur_caractere_scrolling_8x8_TechTech*10)+i];
			if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = tableau_rasters_verticaux_TechTech[j+position_verticale_scrolling_8x8h_TechTech];
			post_dest_verticale = post_dest_verticale + largeur_TechTech;
		}
		pos_dest = pos_dest + 1;
	}
	
	
	
	// puis de (largeur_caractereraster_scrolling_bas_TechTech - pos_dans caracteres ) a largeur_TechTech- posdans caracteres
	
	int nb_lettres_centre = (int) ( largeur_TechTech - (largeur_caractere_scrolling_8x8_TechTech-pos_dans_fonte_scrolling_8x8h_TechTech)) / largeur_caractere_scrolling_8x8_TechTech;
	
	int pos_dans_texte_scrolling_8x8h_TechTech_centre = pos_dans_texte_scrolling_8x8h_TechTech+1;
	
	for (int num_lettre=0;num_lettre < nb_lettres_centre;num_lettre++)
	{
		int lettre = (int) text_scrolltext_8x8h_TechTech[pos_dans_texte_scrolling_8x8h_TechTech_centre];
		lettre = lettre - 32;
		// $ ?
		if (lettre == 4)
		{	pos_dans_texte_scrolling_8x8h_TechTech_centre=0;
			lettre = (int) text_scrolltext_8x8h_TechTech[pos_dans_texte_scrolling_8x8h_TechTech_centre];
			lettre = lettre - 32;
		}
		u32* pointeur_pos_caractere_en_cours_scrolling_8x8h_TechTech_centre = pointeur_fonte_scrolling_8x8_TechTech_u32 + (( lettre / 10 ) * ( 8 * 8 * 10)) + ((lettre % 10) * 8);
		
		for (int i=0;i<largeur_caractere_scrolling_8x8_TechTech;i++)
		{
			int post_dest_verticale = pos_dest;
			for (int j=0;j<hauteur_caractere_scrolling_8x8_TechTech;j++)
			{
				pixel = pointeur_pos_caractere_en_cours_scrolling_8x8h_TechTech_centre[(j*8*10)+i];
				if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = tableau_rasters_verticaux_TechTech[j+position_verticale_scrolling_8x8h_TechTech];
				post_dest_verticale = post_dest_verticale + largeur_TechTech;
			}
			pos_dest = pos_dest + 1;
		}
		pos_dans_texte_scrolling_8x8h_TechTech_centre++;
	}
	
	// puis de largeur_TechTech- posdans caracteres a largeur_TechTech
	// pos_dest on garde
	//
	
	int lettre = (int) text_scrolltext_8x8h_TechTech[pos_dans_texte_scrolling_8x8h_TechTech_centre];
	// TRACE("Lettre droite : %d\n",lettre);
	lettre = lettre - 32;
	// $ ?
	if (lettre == 4)
	{	pos_dans_texte_scrolling_8x8h_TechTech_centre=0;
		lettre = (int) text_scrolltext_8x8h_TechTech[pos_dans_texte_scrolling_8x8h_TechTech_centre];
		lettre = lettre - 32;
	}
	u32* pointeur_pos_caractere_en_cours_scrolling_8x8h_TechTech_centre = pointeur_fonte_scrolling_8x8_TechTech_u32 + (( lettre / 10 ) * ( 8 * 8 * 10)) + ((lettre % 10) * 8);
	
	int largeur_a_afficher_droite = largeur_TechTech-(pos_dest - (largeur_TechTech * position_verticale_scrolling_8x8h_TechTech ));
	
	// TRACE("nb pixel droite : %d\n",largeur_a_afficher_droite);
	
	
	for (int i=0;i<largeur_a_afficher_droite;i++)
	{
		int post_dest_verticale = pos_dest;
		for (int j=0;j<hauteur_caractere_scrolling_8x8_TechTech;j++)
		{
			pixel = pointeur_pos_caractere_en_cours_scrolling_8x8h_TechTech_centre[(j*8*10)+i];
			if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = tableau_rasters_verticaux_TechTech[j+position_verticale_scrolling_8x8h_TechTech];;
			post_dest_verticale = post_dest_verticale + largeur_TechTech;
		}
		pos_dest = pos_dest + 1;
	}
		
		
		
}
	
// scrolling 8x8 bas

static void avance_scrolling_8x8b_TechTech()
{
	// +8
	pos_dans_fonte_scrolling_8x8b_TechTech = pos_dans_fonte_scrolling_8x8b_TechTech + 4;
	
	if (pos_dans_fonte_scrolling_8x8b_TechTech == largeur_caractere_scrolling_8x8_TechTech)
	{
		// nouveau caractere
		pos_dans_fonte_scrolling_8x8b_TechTech = 0;
		pos_dans_texte_scrolling_8x8b_TechTech++;
		int lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech];
		if (lettre == 'a') 
		{
			flag_a=1;
			pos_dans_texte_scrolling_8x8b_TechTech++;
			int lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech];
		}
		if (lettre == 'b') 
		{
			flag_b=1;
			pos_dans_texte_scrolling_8x8b_TechTech++;
			int lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech];
		}
		if (lettre == 'c') 
		{
			flag_c=1;
			pos_dans_texte_scrolling_8x8b_TechTech++;
			int lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech];
		}
		if (lettre == 'd') 
		{
			flag_d=1;
			pos_dans_texte_scrolling_8x8b_TechTech++;
			int lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech];
		}
		
		
		lettre = lettre - 32;
		// $ ?
		if (lettre == 4)
		{	pos_dans_texte_scrolling_8x8b_TechTech=0;
			lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech];
			lettre = lettre - 32;
		}
		pointeur_pos_caractere_en_cours_scrolling_8x8b_TechTech = pointeur_fonte_scrolling_8x8_TechTech_u32 + (( lettre / 10 ) * ( hauteur_caractere_scrolling_8x8_TechTech * largeur_caractere_scrolling_8x8_TechTech * 10)) + ((lettre % 10) * largeur_caractere_scrolling_8x8_TechTech);
	}
			
		
}

static void affiche_scrolling_8x8b_TechTech()
{
	// sur largeur_TechTech de large
	
	// le scrolling a la ligne 4
	int pos_dest = largeur_TechTech * position_verticale_scrolling_8x8b_TechTech ;
	u32	pixel;
	// de 0 a largeur_caractereraster_scrolling_bas_TechTech - pos_dans caracteres
	for (int i=pos_dans_fonte_scrolling_8x8b_TechTech;i<largeur_caractere_scrolling_8x8_TechTech;i++)
	{
		int post_dest_verticale = pos_dest;
		for (int j=0;j<hauteur_caractere_scrolling_8x8_TechTech;j++)
		{
			pixel = pointeur_pos_caractere_en_cours_scrolling_8x8b_TechTech[(j*largeur_caractere_scrolling_8x8_TechTech*10)+i];
			if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = tableau_rasters_verticaux_TechTech[j+position_verticale_scrolling_8x8b_TechTech];
			post_dest_verticale = post_dest_verticale + largeur_TechTech;
		}
		pos_dest = pos_dest + 1;
	}
	
	
	
	// puis de (largeur_caractereraster_scrolling_bas_TechTech - pos_dans caracteres ) a largeur_TechTech- posdans caracteres
	
	int nb_lettres_centre = (int) ( largeur_TechTech - (largeur_caractere_scrolling_8x8_TechTech-pos_dans_fonte_scrolling_8x8b_TechTech)) / largeur_caractere_scrolling_8x8_TechTech;
	
	int pos_dans_texte_scrolling_8x8b_TechTech_centre = pos_dans_texte_scrolling_8x8b_TechTech+1;
	
	for (int num_lettre=0;num_lettre < nb_lettres_centre;num_lettre++)
	{
		int lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech_centre];
		lettre = lettre - 32;
		// $ ?
		if (lettre == 4)
		{	pos_dans_texte_scrolling_8x8b_TechTech_centre=0;
			lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech_centre];
			lettre = lettre - 32;
		}
		u32* pointeur_pos_caractere_en_cours_scrolling_8x8b_TechTech_centre = pointeur_fonte_scrolling_8x8_TechTech_u32 + (( lettre / 10 ) * ( 8 * 8 * 10)) + ((lettre % 10) * 8);
		
		for (int i=0;i<largeur_caractere_scrolling_8x8_TechTech;i++)
		{
			int post_dest_verticale = pos_dest;
			for (int j=0;j<hauteur_caractere_scrolling_8x8_TechTech;j++)
			{
				pixel = pointeur_pos_caractere_en_cours_scrolling_8x8b_TechTech_centre[(j*8*10)+i];
				if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = tableau_rasters_verticaux_TechTech[j+position_verticale_scrolling_8x8b_TechTech];
				post_dest_verticale = post_dest_verticale + largeur_TechTech;
			}
			pos_dest = pos_dest + 1;
		}
		pos_dans_texte_scrolling_8x8b_TechTech_centre++;
	}
	
	// puis de largeur_TechTech- posdans caracteres a largeur_TechTech
	// pos_dest on garde
	//
	
	int lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech_centre];
	// TRACE("Lettre droite : %d\n",lettre);
	lettre = lettre - 32;
	// $ ?
	if (lettre == 4)
	{	pos_dans_texte_scrolling_8x8b_TechTech_centre=0;
		lettre = (int) text_scrolltext_8x8b_TechTech[pos_dans_texte_scrolling_8x8b_TechTech_centre];
		lettre = lettre - 32;
	}
	u32* pointeur_pos_caractere_en_cours_scrolling_8x8b_TechTech_centre = pointeur_fonte_scrolling_8x8_TechTech_u32 + (( lettre / 10 ) * ( 8 * 8 * 10)) + ((lettre % 10) * 8);
	
	int largeur_a_afficher_droite = largeur_TechTech-(pos_dest - (largeur_TechTech * position_verticale_scrolling_8x8b_TechTech ));
	
	// TRACE("nb pixel droite : %d\n",largeur_a_afficher_droite);
	
	
	for (int i=0;i<largeur_a_afficher_droite;i++)
	{
		int post_dest_verticale = pos_dest;
		for (int j=0;j<hauteur_caractere_scrolling_8x8_TechTech;j++)
		{
			pixel = pointeur_pos_caractere_en_cours_scrolling_8x8b_TechTech_centre[(j*8*10)+i];
			if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = tableau_rasters_verticaux_TechTech[j+position_verticale_scrolling_8x8b_TechTech];;
			post_dest_verticale = post_dest_verticale + largeur_TechTech;
		}
		pos_dest = pos_dest + 1;
	}
		
		
		
}
	

// routines scrolling vertical

static void avance_scrolling_vertical_TechTech()
{
	pos_dans_fonte_scrolling_vertical_TechTech = pos_dans_fonte_scrolling_vertical_TechTech + 3;
	if (pos_dans_fonte_scrolling_vertical_TechTech >= (hauteur_caractere_scrolling_vertical_TechTech))
	{
		// nouveau caractere
		pos_dans_fonte_scrolling_vertical_TechTech = 0;
		pos_dans_texte_scrolling_vertical_TechTech++;
		int lettre = (int) text_scrolltext_vertical_TechTech[pos_dans_texte_scrolling_vertical_TechTech];
		lettre = lettre - 32;
		// $ ?
		if (lettre == 4)
		{	pos_dans_texte_scrolling_vertical_TechTech=0;
			lettre = (int) text_scrolltext_bas_TechTech[pos_dans_texte_scrolling_vertical_TechTech];
			lettre = lettre - 32;
		}
		// 10 caractere par ligne
		pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech = pointeur_fonte_scrolling_vertical_TechTech_u32 + (( lettre / 10 ) * ( largeur_caractere_scrolling_vertical_TechTech * hauteur_caractere_scrolling_vertical_TechTech * 10)) + ((lettre % 10) * largeur_caractere_scrolling_vertical_TechTech);
	}
		

		
		
}

static void affiche_scrolling_vertical_TechTech()
{
		// fonte 16x16
		
		int pos_dest = 0;
		u32	pixel;
		int pos_rasters=0;
		
		// en haut a gauche, le caractere en cours
		// de 0 à hauteur_caractere_scrolling_vertical_TechTech-pos_dans_fonte_scrolling_vertical_TechTech
		int pos_dest_horizontale = pos_dest;
		for (int i=pos_dans_fonte_scrolling_vertical_TechTech;i<hauteur_caractere_scrolling_vertical_TechTech;i++)
		{
			
			for (int j=0;j<largeur_caractere_scrolling_vertical_TechTech;j++)
				// 16 de large
			{
				pixel = pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech[(i*160)+j];	
				if (pixel != 0xFF000000) 
				{
					pixel = tableau_rasters_verticaux_TechTech[pos_rasters];
					buffer_ecran_TechTech[pos_dest_horizontale] = pixel;
					buffer_ecran_TechTech[pos_dest_horizontale+32] = pixel;
					buffer_ecran_TechTech[pos_dest_horizontale+64] = pixel;
					
					buffer_ecran_TechTech[pos_dest_horizontale+240] = pixel;
					buffer_ecran_TechTech[pos_dest_horizontale+272] = pixel;
					buffer_ecran_TechTech[pos_dest_horizontale+304] = pixel;
					
				}
				pos_dest_horizontale++;
				
			}
			pos_dest_horizontale=pos_dest_horizontale + largeur_TechTech -largeur_caractere_scrolling_vertical_TechTech ;
			pos_rasters++;
				
		}

		
		
		// de hauteur_caractere_scrolling_vertical_TechTech-pos_dans_fonte_scrolling_vertical_TechTech à 200-pos_dans_fonte_scrolling_vertical_TechTech
		int nb_lettres_centre = (int) ( hauteur_TechTech - ((hauteur_caractere_scrolling_vertical_TechTech) - pos_dans_fonte_scrolling_vertical_TechTech)) / (hauteur_caractere_scrolling_vertical_TechTech);
		//TRACE("nb_lettres_centre : %d\n",nb_lettres_centre);
		
		int pos_dans_texte_scrolling_vertical_TechTech_centre = pos_dans_texte_scrolling_vertical_TechTech+1;
		
		for (int num_lettre=0;num_lettre < nb_lettres_centre;num_lettre++)
		{
			int lettre = (int) text_scrolltext_vertical_TechTech[pos_dans_texte_scrolling_vertical_TechTech_centre];
			lettre = lettre - 32;
			// $ ?
			if (lettre == 4)
			{
			pos_dans_texte_scrolling_vertical_TechTech_centre=0;
			lettre = (int) text_scrolltext_vertical_TechTech[pos_dans_texte_scrolling_vertical_TechTech_centre];
			lettre = lettre - 32;
			}
			u32* pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech_centre = pointeur_fonte_scrolling_vertical_TechTech_u32 + (( lettre / 10 ) * ( hauteur_caractere_scrolling_vertical_TechTech * 160)) + ((lettre % 10) * 16);
		
			for (int i=0;i<(hauteur_caractere_scrolling_vertical_TechTech);i++)
			{
				for (int j=0;j<(largeur_caractere_scrolling_vertical_TechTech);j++)
				{
					pixel = pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech_centre[(i*160)+j];
					if (pixel != 0xFF000000) 
					{
						pixel = tableau_rasters_verticaux_TechTech[pos_rasters];
						buffer_ecran_TechTech[pos_dest_horizontale] = pixel;
						buffer_ecran_TechTech[pos_dest_horizontale+32] = pixel;
						buffer_ecran_TechTech[pos_dest_horizontale+64] = pixel;
					
						buffer_ecran_TechTech[pos_dest_horizontale+240] = pixel;
						buffer_ecran_TechTech[pos_dest_horizontale+272] = pixel;
						buffer_ecran_TechTech[pos_dest_horizontale+304] = pixel;
					}
					pos_dest_horizontale++;
				
				}
				pos_dest_horizontale=pos_dest_horizontale + largeur_TechTech - largeur_caractere_scrolling_vertical_TechTech;
				pos_rasters++;
				
			}
			pos_dans_texte_scrolling_vertical_TechTech_centre++;
		
		}
		
		// de 200-pos_dans_fonte_scrolling_vertical_TechTech  a 200
		int lettre = (int) text_scrolltext_vertical_TechTech[pos_dans_texte_scrolling_vertical_TechTech_centre];
		lettre = lettre - 32;
		// $ ?
		if (lettre == 4)
		{	pos_dans_texte_scrolling_vertical_TechTech_centre=0;
			lettre = (int) text_scrolltext_vertical_TechTech[pos_dans_texte_scrolling_vertical_TechTech_centre];
			lettre = lettre - 32;
		}
		u32* pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech_centre = pointeur_fonte_scrolling_vertical_TechTech_u32 + (( lettre / 10 ) * ( hauteur_caractere_scrolling_vertical_TechTech * 160)) + ((lettre % 10) * 16);
		
		int hauteur_a_afficher_bas = hauteur_TechTech - (pos_dest_horizontale / largeur_TechTech);
		 
		for (int i=0;i<hauteur_a_afficher_bas;i++)
			{
				for (int j=0;j<(largeur_caractere_scrolling_vertical_TechTech);j++)
				{
					pixel = pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech_centre[(i*160)+j];
					if (pixel != 0xFF000000) 
					{
						pixel = tableau_rasters_verticaux_TechTech[pos_rasters];
						buffer_ecran_TechTech[pos_dest_horizontale] = pixel;
						buffer_ecran_TechTech[pos_dest_horizontale+32] = pixel;
						buffer_ecran_TechTech[pos_dest_horizontale+64] = pixel;
					
						buffer_ecran_TechTech[pos_dest_horizontale+240] = pixel;
						buffer_ecran_TechTech[pos_dest_horizontale+272] = pixel;
						buffer_ecran_TechTech[pos_dest_horizontale+304] = pixel;
					}
					pos_dest_horizontale++;
				
				}
				pos_dest_horizontale=pos_dest_horizontale + largeur_TechTech - largeur_caractere_scrolling_vertical_TechTech;
				pos_rasters++;
			}
	
		
}



// scrolling bas

static void avance_scrolling_bas_TechTech()
{
	// +8
	pos_dans_fonte_scrolling_bas_TechTech = pos_dans_fonte_scrolling_bas_TechTech + 6;
	
	if (pos_dans_fonte_scrolling_bas_TechTech == largeur_caractereraster_scrolling_bas_TechTech)
	{
		// nouveau caractere
		pos_dans_fonte_scrolling_bas_TechTech = 0;
		pos_dans_texte_scrolling_bas_TechTech++;
		int lettre = (int) text_scrolltext_bas_TechTech[pos_dans_texte_scrolling_bas_TechTech];
		lettre = lettre - 32;
		// $ ?
		if (lettre == 4)
		{	pos_dans_texte_scrolling_bas_TechTech=0;
			lettre = (int) text_scrolltext_bas_TechTech[pos_dans_texte_scrolling_bas_TechTech];
			lettre = lettre - 32;
		}
		pointeur_pos_caractere_en_cours_scrolling_bas_TechTech = pointeur_fonte_scrolling_bas_TechTech_u32 + (( lettre / 10 ) * ( 99 * 960)) + ((lettre % 10) * 96);
	}
			
		
}


static void affiche_scrolling_bas_TechTech()
{
	// sur largeur_TechTech de large
	// de 0 a largeur_caractereraster_scrolling_bas_TechTech - pos_dans caracteres
	// puis de (largeur_caractereraster_scrolling_bas_TechTech - pos_dans caracteres ) a largeur_TechTech- posdans caracteres
	// puis de largeur_TechTech- posdans caracteres a largeur_TechTech
	
	// le scrolling a la ligne 100
	int pos_dest = largeur_TechTech * 100;
	u32	pixel;
	// de 0 a largeur_caractereraster_scrolling_bas_TechTech - pos_dans caracteres
	//TRACE("pos_dans_fonte_scrolling_bas_TechTech : %d\n",pos_dans_fonte_scrolling_bas_TechTech);
	for (int i=pos_dans_fonte_scrolling_bas_TechTech;i<largeur_caractereraster_scrolling_bas_TechTech;i++)
	{
		//TRACE("a gauche bas i: %d\n",i);
		int post_dest_verticale = pos_dest;
		for (int j=0;j<hauteur_caractereraster_scrolling_bas_TechTech;j++)
		{
			pixel = pointeur_pos_caractere_en_cours_scrolling_bas_TechTech[(j*960)+i];
			if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = pixel;
			post_dest_verticale = post_dest_verticale + largeur_TechTech;
		}
		pos_dest = pos_dest + 1;
	}
	
	
	
	// puis de (largeur_caractereraster_scrolling_bas_TechTech - pos_dans caracteres ) a largeur_TechTech- posdans caracteres
	
	int nb_lettres_centre = (int) ( largeur_TechTech - (largeur_caractereraster_scrolling_bas_TechTech-pos_dans_fonte_scrolling_bas_TechTech)) / largeur_caractereraster_scrolling_bas_TechTech;
	//TRACE("nb_lettres_centre : %d\n",nb_lettres_centre);
	
	int pos_dans_texte_scrolling_bas_TechTech_centre = pos_dans_texte_scrolling_bas_TechTech+1;
	
	for (int num_lettre=0;num_lettre < nb_lettres_centre;num_lettre++)
	{
		int lettre = (int) text_scrolltext_bas_TechTech[pos_dans_texte_scrolling_bas_TechTech_centre];
		lettre = lettre - 32;
		// $ ?
		if (lettre == 4)
		{	pos_dans_texte_scrolling_bas_TechTech_centre=0;
			lettre = (int) text_scrolltext_bas_TechTech[pos_dans_texte_scrolling_bas_TechTech_centre];
			lettre = lettre - 32;
		}
		u32* pointeur_pos_caractere_en_cours_scrolling_bas_TechTech_centre = pointeur_fonte_scrolling_bas_TechTech_u32 + (( lettre / 10 ) * ( 99 * 960)) + ((lettre % 10) * 96);
		
		for (int i=0;i<largeur_caractereraster_scrolling_bas_TechTech;i++)
		{
			int post_dest_verticale = pos_dest;
			for (int j=0;j<hauteur_caractereraster_scrolling_bas_TechTech;j++)
			{
				pixel = pointeur_pos_caractere_en_cours_scrolling_bas_TechTech_centre[(j*960)+i];
				if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = pixel;
				post_dest_verticale = post_dest_verticale + largeur_TechTech;
			}
			pos_dest = pos_dest + 1;
		}
		pos_dans_texte_scrolling_bas_TechTech_centre++;
	}
	
	// puis de largeur_TechTech- posdans caracteres a largeur_TechTech
	// pos_dest on garde
	//
	
	int lettre = (int) text_scrolltext_bas_TechTech[pos_dans_texte_scrolling_bas_TechTech_centre];
	// TRACE("Lettre droite : %d\n",lettre);
	lettre = lettre - 32;
	// $ ?
	if (lettre == 4)
	{	pos_dans_texte_scrolling_bas_TechTech_centre=0;
		lettre = (int) text_scrolltext_bas_TechTech[pos_dans_texte_scrolling_bas_TechTech_centre];
		lettre = lettre - 32;
	}
	u32* pointeur_pos_caractere_en_cours_scrolling_bas_TechTech_centre = pointeur_fonte_scrolling_bas_TechTech_u32 + (( lettre / 10 ) * ( 99 * 960)) + ((lettre % 10) * 96);
	
	int largeur_a_afficher_droite = largeur_TechTech-(pos_dest - (largeur_TechTech * 100));
	
	// TRACE("nb pixel droite : %d\n",largeur_a_afficher_droite);
	
	
	for (int i=0;i<largeur_a_afficher_droite;i++)
	{
		int post_dest_verticale = pos_dest;
		for (int j=0;j<hauteur_caractereraster_scrolling_bas_TechTech;j++)
		{
			pixel = pointeur_pos_caractere_en_cours_scrolling_bas_TechTech_centre[(j*960)+i];
			if (pixel != 0x00000000) buffer_ecran_TechTech[post_dest_verticale] = pixel;
			post_dest_verticale = post_dest_verticale + largeur_TechTech;
		}
		pos_dest = pos_dest + 1;
	}
		
		
		
}
	


static void affiche_sprite_TechTech(int numero_sprite, int x_sprite, int y_sprite)
{
	int pos_source = numero_sprite * ecart_entre_sprite_dans_png_TechTech;
	int pos_dest = x_sprite + ( y_sprite * largeur_TechTech);
	u32 pixel;
	for (int y=0;y<hauteur_sprite_TechTech;y++)
	{
		for (int x=0;x<largeur_sprite_TechTech; x++)
		{
			pixel = pointeur_sprites_TechTech_u32[pos_source];
			// pixel = pixel & 0x00FFFFFF;
			if (pixel != 0x00000000) buffer_ecran_TechTech[pos_dest] = pixel;
			pos_source++;
			pos_dest++;
		}
		pos_source = pos_source + largeur_bande_sprite_TechTech - largeur_sprite_TechTech;
		pos_dest = pos_dest + largeur_TechTech - largeur_sprite_TechTech;
	}
	
	
}


static void affiche_tous_les_sprites_TechTech()
{
		if (ychange>25) addy=-0.2f;
		if (ychange<-25) addy=0.2f;
		ychange+=addy;
		swing += 0.04;
		swingy += 0.05;
		
		siny=ychange*sin(swingy);
		
		int numsprites_Tech = 0;
		for (int i=0;i < nombre_sprites_TechTech+1;i++)
		{
			if (i != 3)
			{
				x_sprite_en_cours = (int) (spx + (145.0f * cos(swing - ( 0.1f*i))));
				y_sprite_en_cours = (int) (spy + (ychange * sin(swingy-(0.2f*i))) + siny);

				affiche_sprite_TechTech(numsprites_Tech,x_sprite_en_cours,y_sprite_en_cours);
				numsprites_Tech++;
			}
		}
}

static void copie_fond_vert_TechTech(int x_fond_vert, int y_fond_vert)
{
	if (x_fond_vert >=0 ) x_fond_vert = abs(x_fond_vert % largeur_fond_vert_TechTech);
	if (x_fond_vert<0) x_fond_vert = largeur_fond_vert_TechTech - abs(x_fond_vert % largeur_fond_vert_TechTech);

	if (y_fond_vert >=0 ) y_fond_vert = abs(y_fond_vert % hauteur_fond_vert_TechTech);
	if (y_fond_vert<0) y_fond_vert = hauteur_fond_vert_TechTech - abs(y_fond_vert % hauteur_fond_vert_TechTech);
	
	
	int pos_source_fondvert = 0;
	int pos_dest_fondvert = (y_fond_vert*largeur_TechTech)+x_fond_vert;
	for (int y=y_fond_vert;y<hauteur_TechTech;y++)
	{
		for (int x=x_fond_vert;x<largeur_fond_vert_TechTech;x++)
		{
			buffer_ecran_TechTech[pos_dest_fondvert] = pointeur_fond_vert_TechTech_u32[pos_source_fondvert];
			pos_source_fondvert++;
			pos_dest_fondvert++;
		}
		pos_source_fondvert = pos_source_fondvert + largeur_fond_vert_TechTech-(largeur_fond_vert_TechTech-x_fond_vert);
		pos_dest_fondvert = pos_dest_fondvert + largeur_TechTech - (largeur_fond_vert_TechTech-x_fond_vert);
		
	}
	// en bas a gauche
	pos_source_fondvert = ( largeur_fond_vert_TechTech - x_fond_vert);
	pos_dest_fondvert = (y_fond_vert*largeur_TechTech);
	for (int y=y_fond_vert;y<hauteur_TechTech;y++)
	{
		for (int x=0;x<x_fond_vert;x++)
		{
			buffer_ecran_TechTech[pos_dest_fondvert] = pointeur_fond_vert_TechTech_u32[pos_source_fondvert];
			pos_source_fondvert++;
			pos_dest_fondvert++;
		}
		pos_source_fondvert = pos_source_fondvert + largeur_fond_vert_TechTech-x_fond_vert;
		pos_dest_fondvert = pos_dest_fondvert + largeur_TechTech - x_fond_vert;
	}
	// en haut a droite
	pos_source_fondvert = ((hauteur_fond_vert_TechTech-y_fond_vert) * largeur_fond_vert_TechTech);
	pos_dest_fondvert = x_fond_vert;
	for (int y=0;y<(y_fond_vert);y++)
	{
		for (int x=x_fond_vert;x<largeur_fond_vert_TechTech;x++)
		{
			buffer_ecran_TechTech[pos_dest_fondvert] = pointeur_fond_vert_TechTech_u32[pos_source_fondvert];
			pos_source_fondvert++;
			pos_dest_fondvert++;
		}
		pos_source_fondvert = pos_source_fondvert + largeur_fond_vert_TechTech-(largeur_fond_vert_TechTech-x_fond_vert);
		pos_dest_fondvert = pos_dest_fondvert + largeur_TechTech - (largeur_fond_vert_TechTech-x_fond_vert);
	}
	// en haut a gauche
	pos_source_fondvert = ((hauteur_fond_vert_TechTech-y_fond_vert) * largeur_fond_vert_TechTech) + (largeur_fond_vert_TechTech-x_fond_vert);
	pos_dest_fondvert = 0;
	for (int y=0;y<(y_fond_vert);y++)
	{
		for (int x=0;x<x_fond_vert;x++)
		{
			buffer_ecran_TechTech[pos_dest_fondvert] = pointeur_fond_vert_TechTech_u32[pos_source_fondvert];
			pos_source_fondvert++;
			pos_dest_fondvert++;
		}
		pos_source_fondvert = pos_source_fondvert + largeur_fond_vert_TechTech-x_fond_vert;
		pos_dest_fondvert = pos_dest_fondvert + largeur_TechTech - x_fond_vert;
	}
	
}
	


static void copie_fond_rose_TechTech(int x_fond_rose, int y_fond_rose)
{
	if (x_fond_rose >=0 ) x_fond_rose = abs(x_fond_rose % largeur_fond_rose_TechTech);
	if (x_fond_rose<0) x_fond_rose = largeur_fond_rose_TechTech - abs(x_fond_rose % largeur_fond_rose_TechTech);

	if (y_fond_rose >=0 ) y_fond_rose = abs(y_fond_rose % hauteur_fond_rose_TechTech);
	if (y_fond_rose<0) y_fond_rose = hauteur_fond_rose_TechTech - abs(y_fond_rose % hauteur_fond_rose_TechTech);
	
	
	//x_fond_rose = abs(x_fond_rose % largeur_fond_rose_TechTech);
	//y_fond_rose = abs(y_fond_rose % hauteur_fond_rose_TechTech);
	
	// en bas a droite
	u32	pixel;
	int pos_source_fondrose = 0;
	int pos_dest_fondrose = (y_fond_rose*largeur_TechTech)+x_fond_rose;
	// pixel = pointeur_fond_rose_TechTech_u32[pos_source_fondrose];
	// TRACE("pixel : %X\n",pixel);
	for (int y=y_fond_rose;y<hauteur_TechTech;y++)
	{
		for (int x=x_fond_rose;x<largeur_fond_rose_TechTech;x++)
		{
			
			pixel = pointeur_fond_rose_TechTech_u32[pos_source_fondrose];
			if (pixel !=0xFF000000) buffer_ecran_TechTech[pos_dest_fondrose] = pixel;
			pos_source_fondrose++;
			pos_dest_fondrose++;
		}
		pos_source_fondrose = pos_source_fondrose + largeur_fond_rose_TechTech-(largeur_fond_rose_TechTech-x_fond_rose);
		pos_dest_fondrose = pos_dest_fondrose + largeur_TechTech - (largeur_fond_rose_TechTech-x_fond_rose);
		
	}
	// en bas a gauche
	pos_source_fondrose = ( largeur_fond_rose_TechTech - x_fond_rose);
	pos_dest_fondrose = (y_fond_rose*largeur_TechTech);
	for (int y=y_fond_rose;y<hauteur_TechTech;y++)
	{
		for (int x=0;x<x_fond_rose;x++)
		{
			pixel = pointeur_fond_rose_TechTech_u32[pos_source_fondrose];
			if (pixel !=0xFF000000) buffer_ecran_TechTech[pos_dest_fondrose] = pixel;
			pos_source_fondrose++;
			pos_dest_fondrose++;
		}
		pos_source_fondrose = pos_source_fondrose + largeur_fond_rose_TechTech-x_fond_rose;
		pos_dest_fondrose = pos_dest_fondrose + largeur_TechTech - x_fond_rose;
	}
	// en haut a droite
	pos_source_fondrose = ((hauteur_fond_rose_TechTech-y_fond_rose) * largeur_fond_rose_TechTech);
	pos_dest_fondrose = x_fond_rose;
	for (int y=0;y<(y_fond_rose);y++)
	{
		for (int x=x_fond_rose;x<largeur_fond_rose_TechTech;x++)
		{
			pixel = pointeur_fond_rose_TechTech_u32[pos_source_fondrose];
			if (pixel !=0xFF000000) buffer_ecran_TechTech[pos_dest_fondrose] = pixel;
			pos_source_fondrose++;
			pos_dest_fondrose++;
		}
		pos_source_fondrose = pos_source_fondrose + largeur_fond_rose_TechTech-(largeur_fond_rose_TechTech-x_fond_rose);
		pos_dest_fondrose = pos_dest_fondrose + largeur_TechTech - (largeur_fond_rose_TechTech-x_fond_rose);
	}
	// en haut a gauche
	pos_source_fondrose = ((hauteur_fond_rose_TechTech-y_fond_rose) * largeur_fond_rose_TechTech) + (largeur_fond_rose_TechTech-x_fond_rose);
	pos_dest_fondrose = 0;
	for (int y=0;y<(y_fond_rose);y++)
	{
		for (int x=0;x<x_fond_rose;x++)
		{
			pixel = pointeur_fond_rose_TechTech_u32[pos_source_fondrose];
			if (pixel !=0xFF000000) buffer_ecran_TechTech[pos_dest_fondrose] = pixel;
			pos_source_fondrose++;
			pos_dest_fondrose++;
		}
		pos_source_fondrose = pos_source_fondrose + largeur_fond_rose_TechTech-x_fond_rose;
		pos_dest_fondrose = pos_dest_fondrose + largeur_TechTech - x_fond_rose;
	}
	
}

// deplace les fonds
static void deplace_fonds_Tech_Tech()
{
		x_fond_rose_TechTech = x_fond_rose_TechTech + increment_fond_rose_x;
		y_fond_rose_TechTech = y_fond_rose_TechTech + increment_fond_rose_y;
	
		if (x_fond_rose_TechTech == -512 && y_fond_rose_TechTech==0)
		{
				increment_fond_rose_x=0;
				increment_fond_rose_y = -2;
		}
	
		if (y_fond_rose_TechTech == -192 && x_fond_rose_TechTech == -512 )
		{
				increment_fond_rose_x = 16;
				increment_fond_rose_y = 0;
		}
	
		if (x_fond_rose_TechTech == 0  && y_fond_rose_TechTech == -192)
		{
			increment_fond_rose_x=0;
			increment_fond_rose_y = 2;
		}
	
		if (y_fond_rose_TechTech ==0 && x_fond_rose_TechTech == 0)
		{
				increment_fond_rose_x = -16;
				increment_fond_rose_y = 0;
		}	
	
		x_fond_vert_TechTech = x_fond_vert_TechTech + increment_fond_vert_x;
		y_fond_vert_TechTech = y_fond_vert_TechTech + increment_fond_vert_y+increment_fond_rose_y;
	
		if (x_fond_vert_TechTech == 0 && y_fond_vert_TechTech<=0)
		{
				increment_fond_vert_x=+16;
				increment_fond_vert_y = 0;
		}

		if (x_fond_vert_TechTech == 432 && y_fond_vert_TechTech<=0)
		{
				increment_fond_vert_x=0;
				increment_fond_vert_y = 1;
		}

		if (x_fond_vert_TechTech == 432 && y_fond_vert_TechTech>=50)
		{
				increment_fond_vert_x=-16;
				increment_fond_vert_y = 0;
		}

		if (x_fond_vert_TechTech == 0 && y_fond_vert_TechTech>=50)
		{
				increment_fond_vert_x=0;
				increment_fond_vert_y = -1;
		}	

}



// routines megascroll ULM


// routines megascroll ULM
static void copie_fond()
{
	// 544 lignes = 32 motifs verticaux
	// decalage en X, 16 octets par ligne 
	// relatif à $49d2, table de motif en couleur
	int valeur_dec1 = table_2_valeurs_w[pointeur_pos_table_2_valeurs_w+1];
	int valeur_dec2 = table_2_valeurs_w[pointeur_pos_table_2_valeurs_w+3];
	//TRACE("valeur_dec1 : %X\n",valeur_dec1);
	//TRACE("valeur_dec2 : %X\n",valeur_dec2);
	
	// divisé par 16 ( 16 octets par ligne )
	int decalage_en_x_pour_toutes_les_lignes = valeur_dec1 >> 4;
	// divisé par 2
	// decalage_en_x_pour_toutes_les_lignes = decalage_en_x_pour_toutes_les_lignes >> 1;
	
	// décalage en Y
	float sinus = sin(radian_mouvement_fond_vertical_megascroller_ULM);
	int decalage_y = (int) (sinus * 20.0f);
	decalage_y = decalage_y + 20;
	decalage_y = decalage_y % hauteur_tile_megascroll_ULM;
	//TRACE("decalage_y : %d\n", decalage_y);
	decalage_y = decalage_y * ( largeur_tile_megascroll_ULM + largeur_megascroll_ULM);
	
	
	// division ? 
	valeur_dec2 = valeur_dec2 >>1;
	
	
	int pos_decalage = decalage_en_cours_horizontal_megascroll_ULM ;
	int decalage_x_source=0;
	int	pos_dest = 0;
	int pos_source;
	for (int i=0;i< ( hauteur_megascroll_ULM / hauteur_tile_megascroll_ULM);i++)
	{
		pos_source = 0;
		for (int y=0;y<hauteur_tile_megascroll_ULM;y++)
		{
			decalage_x_source= (u16) courbe_horizontale_ULM2w[pos_decalage+1];
			decalage_x_source = decalage_x_source >> 1;
			decalage_x_source = decalage_x_source  + decalage_en_x_pour_toutes_les_lignes;
			decalage_x_source = decalage_x_source % largeur_tile_megascroll_ULM;
			//TRACE("decalage en x : %d\n",decalage_x_source);
			pos_decalage++;
			pos_decalage++;
			
			if (pos_decalage > courbe_horizontale_ULM2w_size-2-80)
			{
				pos_decalage = 0;
			}
			
			for (int x=0;x < largeur_megascroll_ULM;x++)
			{
				buffer_ecran_megascroll_ULM[pos_dest] = ligne_tile_megascroll_ULM[decalage_x_source + pos_source+decalage_y];
				//buffer_ecran_megascroll_ULM[pos_dest] = ligne_tile_megascroll_ULM[decalage_x_source+ pos_source];
				
				pos_dest++;
				pos_source++;
			}
		// on saute le bord 
		pos_source= pos_source + largeur_tile_megascroll_ULM;
		}
	}
			
}

static void affiche_megascroll_ULM()
{
	// calcul a quelle ligne il faut afficher
	int	position_en_y_du_scrolling = (int) (sin(radian_mouvement_bigscroll_megascroller_ULM) *36)+10;
	//TRACE("position_en_y_du_scrolling : %d\n",position_en_y_du_scrolling);
	// de -31 a 31
	

	// recuperer un caractere
	u8 lettre = (u8) scroll_text_megascroll_ULM[pos_text_scroller_megascroll_ULM];
	// position dans la font
	lettre = lettre - 32;
	int ligne = lettre / 16;
	int colonne = lettre - (ligne * 16 );

	// TRACE("lettre : %d\n", lettre);
	// TRACE("ligne : %d\n", ligne);
	// TRACE("colonne : %d\n", colonne);

	// on saute N lignes de fontes
	int position_1ere_colonne_lettre = ( ligne * longeur_ligne_font_megascroll_ULM  * hauteur_1font_megascroll_ULM);
	// on saute N colonnes de fontes
	position_1ere_colonne_lettre = position_1ere_colonne_lettre + ( colonne * largeur_1font_megascroll_ULM);

	// suivant la position ou on en est dans la lettre en cours:
	
	int	pos_actuelle_colonne_dans_font = pos_dans_font_scroller_megascroll_ULM;
	int pos_actuelle_text_scroller_megascroll_ULM = pos_text_scroller_megascroll_ULM;

	int pos_source_ball = 0;
	int	pos_dest_ball = 0;
	for (int pos_x_destination=0;pos_x_destination < largeur_megascroll_ULM; pos_x_destination = pos_x_destination +16)
	
	{
		// il faut lire les pixels de la fonte
		// et afficher une boule si <> 0
		int	pos_source = position_1ere_colonne_lettre+pos_actuelle_colonne_dans_font;
		
		
		int pos_destination_bigscroll = pos_x_destination + ( position_en_y_du_scrolling * largeur_megascroll_ULM) ;
		for (int y=0;y<hauteur_1font_megascroll_ULM;y++)
		{
			//TRACE("pixel font : %X\n",pointeur_fnt_u32_megascroller_ULM[pos_source]);
			if (pointeur_fnt_u32_megascroller_ULM[pos_source] == 0xFFFFFFFF)
			{
				// le pixel n'est pas vide, on doit afficher une boule
				// dim 16x16
				// 4 points pour l'instant
				pos_source_ball = 0;
				pos_dest_ball = pos_destination_bigscroll;
				
				for (int j=0;j<16;j++)
				{
						
						// on est dans l ecran
						for (int i=0;i<16;i++)
						{	
							u32	pixel = pointeur_ball_u32_megascroller_ULM[pos_source_ball];
							if (pixel !=0xFF000000) buffer_ecran_megascroll_ULM[pos_dest_ball]=pixel;
							// buffer_ecran_megascroll_ULM[pos_dest_ball]=pointeur_ball_u32_megascroller_ULM[pos_source_ball];
							pos_dest_ball++;
							pos_source_ball++;
						}
						pos_dest_ball = pos_dest_ball + largeur_megascroll_ULM - 16;
					
				}
				
			}
			// ligne suivant dans la fonte
			pos_source = pos_source + longeur_ligne_font_megascroll_ULM;
			// 16 lignes plus bas sur l'ecran
			pos_destination_bigscroll = pos_destination_bigscroll + 16 * largeur_megascroll_ULM;
		}
		// colonne suivante :
		pos_actuelle_colonne_dans_font = pos_actuelle_colonne_dans_font+1;
		if (pos_actuelle_colonne_dans_font==16)
		{
			pos_actuelle_text_scroller_megascroll_ULM = pos_actuelle_text_scroller_megascroll_ULM + 1;
			// recuperer un caractere
			lettre = (u8) scroll_text_megascroll_ULM[pos_actuelle_text_scroller_megascroll_ULM];
			//TRACE("nouvelle lettre : %d\n", lettre);
			// $ ?
			if (lettre == 36 )
			{
				pos_actuelle_text_scroller_megascroll_ULM=0;
				lettre = (u8) scroll_text_megascroll_ULM[pos_actuelle_text_scroller_megascroll_ULM];
			}
			// position dans la font
			lettre = lettre - 32;
			ligne = lettre / 16;
			colonne = lettre - (ligne * 16 );
			// on saute N lignes de fontes
			position_1ere_colonne_lettre = ( ligne * longeur_ligne_font_megascroll_ULM  * hauteur_1font_megascroll_ULM);
			// on saute N colonnes de fontes
			position_1ere_colonne_lettre = position_1ere_colonne_lettre + ( colonne * largeur_1font_megascroll_ULM);
			
			pos_actuelle_colonne_dans_font=0;
		}
			
	}
	
	
}

static void avance_scroller()
{
	radian_mouvement_bigscroll_megascroller_ULM = radian_mouvement_bigscroll_megascroller_ULM + 0.05f;
	pos_dans_font_scroller_megascroll_ULM = pos_dans_font_scroller_megascroll_ULM + 1;
	//TRACE("on avance");
	if (pos_dans_font_scroller_megascroll_ULM == 16)
	{
		pos_text_scroller_megascroll_ULM = pos_text_scroller_megascroll_ULM + 1;
		// recuperer un caractere
		u8 lettre = (u8) scroll_text_megascroll_ULM[pos_text_scroller_megascroll_ULM];
		//TRACE("nouvelle lettre : %d\n", lettre);
		// $ ?
		if (lettre == 36 )
		{
			pos_text_scroller_megascroll_ULM=0;
		}
		pos_dans_font_scroller_megascroll_ULM=0;
			
	}
}



// routines de gestion des scrollers du fullscreen
static void display_scrollers(int numero_ecran_parallax)
{
	/*
	- incrementer la position dans la lettre
	- si = largeur de la lettre, nouvelle lettre
		- recuperer la nouvelle lettre
		- position dans la lettre à 0
	- ecrire une bande de 2 pixels de largeurs Y dans pointeur_buffer_ecrans_scrolling +  (numero_ecran_parallax*largeur_fullscreen_TCB*2*hauteur_fullscreen_TCB) + position actuel du scrolling 
	- la meme bande de 2 pixels dans le pointeur + 1 ecran  : + largeur_fullscreen_TCB
	*/
	
	int ligne_premier_scrolling = ligne_debut_remplissage_fond;
	
	
	for (int numero_scrolling = 0; numero_scrolling<nombre_scrollers_fullscreen_TCB-1;numero_scrolling++)
	{
		if (scrollers_fullscreen_TCB[numero_scrolling].delai_avant_demarrage < iteration)
		{
			if (scrollers_fullscreen_TCB[numero_scrolling].position_dans_le_caractere_actuel == 0)
			{	// nouvelle lettre
				char* pointeur_actuel_dans_texte = scrollers_fullscreen_TCB[numero_scrolling].pointeur_en_cours_dans_le_message;
				char caractere = pointeur_actuel_dans_texte[0];
				if (caractere == 'e')
				{	// fin du texte du message
					scrollers_fullscreen_TCB[numero_scrolling].pointeur_en_cours_dans_le_message = scrollers_fullscreen_TCB[numero_scrolling].pointeur_vers_debut_message;
					caractere  = scrollers_fullscreen_TCB[numero_scrolling].pointeur_en_cours_dans_le_message[0];
					scrollers_fullscreen_TCB[numero_scrolling].pointeur_en_cours_dans_le_message = scrollers_fullscreen_TCB[numero_scrolling].pointeur_en_cours_dans_le_message+1;
				}
				else
				{
					scrollers_fullscreen_TCB[numero_scrolling].pointeur_en_cours_dans_le_message = scrollers_fullscreen_TCB[numero_scrolling].pointeur_en_cours_dans_le_message+1;
				}
				
				// par defaut 'and'
				int pos_lettre_x = 336+44;
				int pos_lettre_y = 200;
				int pos_lettre_largeur = 32;
				// recherche dans la table des correspondances avec la position dans l'image des fonts
				for (int i=0;i<30;i++)
				{
					if (letters_fullscreen_TCB[i].lettre == caractere)
					{
						pos_lettre_x = letters_fullscreen_TCB[i].x;
						pos_lettre_y = letters_fullscreen_TCB[i].y;
						pos_lettre_largeur = letters_fullscreen_TCB[i].largeur;
						//TRACE("iteration : %d\n",iteration);
						//TRACE("numero scrolling : %d\n",numero_scrolling);
						//TRACE("pos_lettre_x : %d\n",pos_lettre_x);
						//TRACE("pos_lettre_y : %d\n",pos_lettre_y);
						//TRACE("pos_lettre_largeur : %d\n",pos_lettre_largeur);
						break;
					}	
				}
				u32* pointeur_memoire_sur_la_fonte = fonts + ( pos_lettre_y * 420 ) + pos_lettre_x;
				scrollers_fullscreen_TCB[numero_scrolling].pointeur_memoire_sur_font = pointeur_memoire_sur_la_fonte;
				scrollers_fullscreen_TCB[numero_scrolling].largeur_caractere_actuel = pos_lettre_largeur;
			}
			
			int hauteur_a_remplir =0;
			if (hauteur_fullscreen_TCB - ligne_debut_remplissage_fond - ( numero_scrolling * nombre_lignes_par_lettre_scroller_fullscreen_TCB) >40 ) 
				{
					hauteur_a_remplir = 40;
				}
			else
				{
					hauteur_a_remplir = hauteur_fullscreen_TCB - ligne_debut_remplissage_fond - ( numero_scrolling * nombre_lignes_par_lettre_scroller_fullscreen_TCB);
				}
			
			
			// afficher a partir de .pointeur_memoire_sur_font + position_dans_le_caractere_actuel
			// vers 
			// pointeur_buffer_ecrans_scrolling
			
			u32* pointeur_source_font = scrollers_fullscreen_TCB[numero_scrolling].pointeur_memoire_sur_font;
			pointeur_source_font = pointeur_source_font + scrollers_fullscreen_TCB[numero_scrolling].position_dans_le_caractere_actuel;
			
			u32* pointeur_dest_ecran_scrolling_gauche = pointeur_buffer_ecrans_scrolling;
			// on saute 24 lignes ( logo en haut de l'ecran )
			pointeur_dest_ecran_scrolling_gauche = pointeur_dest_ecran_scrolling_gauche + (ligne_debut_remplissage_fond*largeur_fullscreen_TCB*2);
			
			// pointeur_dest_ecran_scrolling_gauche = pointeur_dest_ecran_scrolling_gauche + numero_ecran_parallax * nb_pixels_a_faire_entrer_par_vbl_vlb_fullscreen_TCB;
			
			// on saute 40 lignes par ligne de scrolling
			pointeur_dest_ecran_scrolling_gauche = pointeur_dest_ecran_scrolling_gauche + ( numero_scrolling * largeur_fullscreen_TCB * 2 * 40 );
			// on prend en compte le scrolling horizontal
			pointeur_dest_ecran_scrolling_gauche = pointeur_dest_ecran_scrolling_gauche + pos_scrolling_horizontal_dans_ecran;
			
			// a droite aussi
			u32* pointeur_dest_ecran_scrolling_droite = pointeur_dest_ecran_scrolling_gauche + largeur_fullscreen_TCB;
			
			for (int numero_ecran_parallax_scrollers = 0;numero_ecran_parallax_scrollers<nb_ecrans_fullscreen_TCB;numero_ecran_parallax_scrollers++)
			{
								
				for (int y=0;y<hauteur_a_remplir;y++)
				{
					for (int x=0;x<nb_pixels_a_faire_entrer_par_vbl_vlb_fullscreen_TCB;x++)
					{
						u32 pixelfinal = back_rgba[( ( x - numero_ecran_parallax_scrollers + 8 + pos_scrolling_horizontal_dans_ecran+ ligne_debut_remplissage_fond*largeur_fullscreen_TCB*2)  % largeur_motif_fond_fullscreen_TCB ) + ( (y % hauteur_motif_fond_fullscreen_TCB)  * largeur_motif_fond_fullscreen_TCB)];
						u32 pixelfont = pointeur_source_font[ x + ( y * 420) ];
						if (pixelfont != 0xFF000000) pixelfinal = pixelfont;
						pointeur_dest_ecran_scrolling_gauche[x + (y*largeur_fullscreen_TCB*2)] = pixelfinal;
						pointeur_dest_ecran_scrolling_droite[x + (y*largeur_fullscreen_TCB*2)] = pixelfinal;
					}
					
				}
				
				pointeur_dest_ecran_scrolling_gauche = pointeur_dest_ecran_scrolling_gauche + (largeur_fullscreen_TCB*2*hauteur_fullscreen_TCB);
				pointeur_dest_ecran_scrolling_droite = pointeur_dest_ecran_scrolling_droite + (largeur_fullscreen_TCB*2*hauteur_fullscreen_TCB);
				
			}
			
					
			scrollers_fullscreen_TCB[numero_scrolling].position_dans_le_caractere_actuel = scrollers_fullscreen_TCB[numero_scrolling].position_dans_le_caractere_actuel + nb_pixels_a_faire_entrer_par_vbl_vlb_fullscreen_TCB;
			if (scrollers_fullscreen_TCB[numero_scrolling].position_dans_le_caractere_actuel == scrollers_fullscreen_TCB[numero_scrolling].largeur_caractere_actuel)
			{
				scrollers_fullscreen_TCB[numero_scrolling].position_dans_le_caractere_actuel = 0;
			}
				
		}
		
	}
}	
	

	
	



// routines d'affichage des sprites

static void efface_sprite_fullscreen_TCB(int numero_ecran_parallax,int numero_de_sprite)
{
	// on commence par restaurer
	u32* pointeur_ecran_dest_restauration = sprites[numero_de_sprite].pointeur_destination_precedents;
	u32* pointeur_buffer_sauvegarde_CLS = sprites[numero_de_sprite].buffers_cls;

	for(int y=0;y<hauteur_sprite_fullscreen_TCB;y++)
	{
		for(int x=0;x<largeur_sprite_fullscreen_TCB;x++)
		{
			pointeur_ecran_dest_restauration[0]=pointeur_buffer_sauvegarde_CLS[0];
			pointeur_ecran_dest_restauration++;
			pointeur_buffer_sauvegarde_CLS++;
		}
		pointeur_ecran_dest_restauration = pointeur_ecran_dest_restauration + largeur_fullscreen_TCB*2 - largeur_sprite_fullscreen_TCB;
	}
}

static void sauvegarde_sprite_fullscreen_TCB(int numero_ecran_parallax, int numero_de_sprite)
{
	// on sauvegarde le fond et son pointeur calculé
	int x = sprites[numero_de_sprite].x_en_cours;
	int y = sprites[numero_de_sprite].y_en_cours;
	
	u32* pointeur_source_sauvegarde = pointeur_buffer_ecrans_scrolling + (numero_ecran_parallax * largeur_fullscreen_TCB * hauteur_fullscreen_TCB * 2);
	// 
	pointeur_source_sauvegarde = pointeur_source_sauvegarde + x + (y* largeur_fullscreen_TCB*2)+pos_scrolling_horizontal_dans_ecran;
	// u32* pointeur_dest_sprite = pointeur_source_sauvegarde;
	//TRACE("pointeur_source_sauvegarde %X\n", pointeur_source_sauvegarde);
	sprites[numero_de_sprite].pointeur_destination_precedents = pointeur_source_sauvegarde;

	// buffer de stockage du fond
	u32* pointeur_sprite_dest_sauvegarde = sprites[numero_de_sprite].buffers_cls;

	for(int y=0;y<hauteur_sprite_fullscreen_TCB;y++)
	{
		for(int x=0;x<largeur_sprite_fullscreen_TCB;x++)
		{
			pointeur_sprite_dest_sauvegarde[0] = pointeur_source_sauvegarde[0];
			pointeur_source_sauvegarde++;
			pointeur_sprite_dest_sauvegarde++;
		}
		pointeur_source_sauvegarde = pointeur_source_sauvegarde + (largeur_fullscreen_TCB*2) - largeur_sprite_fullscreen_TCB;
	}



}

static void affiche_sprite_fullscreen_TCB(int numero_de_sprite)
{
	// TRACE("affichage sprite %d\n", numero_de_sprite);
	//int x = sprites[numero_de_sprite].x_en_cours;
	//int y = sprites[numero_de_sprite].y_en_cours;

	u32* pointeur_dest_sprite = sprites[numero_de_sprite].pointeur_destination_precedents;

	// affichage du sprite
	// destination = pointeur_dest_sprite

	u32* pointeur_source_sprite = sprites[numero_de_sprite].dessin_sprite;
	
	//TRACE("pointeur_source_sprite %X\n", pointeur_source_sprite);
	//TRACE("pointeur_dest_sprite %X\n", pointeur_dest_sprite);
	
	
	int pos_destination = 0;
	int pos_source=0;
	for(int y=0;y<hauteur_sprite_fullscreen_TCB;y++)
	{
		for(int x=0;x<largeur_sprite_fullscreen_TCB;x++)
		{
			u32 pixel = pointeur_source_sprite[pos_source];
			pos_source++;
				if (( pixel & 0xFFFFFF) != 0)
				{
					pointeur_dest_sprite[pos_destination] = pixel;
				}
			pos_destination++;


			}
			pos_destination = pos_destination + (largeur_fullscreen_TCB*2) - largeur_sprite_fullscreen_TCB;
		}
}





// OPENGL
//-----------------------------------------------------------------------------
// EGL initialization
//-----------------------------------------------------------------------------

static EGLDisplay s_display;
static EGLContext s_context;
static EGLSurface s_surface;

static bool initEgl(NWindow* win)
{
    // Connect to the EGL default display
    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!s_display)
    {
        TRACE("Could not connect to display! error: %d", eglGetError());
        goto _fail0;
    }

    // Initialize the EGL display connection
    eglInitialize(s_display, nullptr, nullptr);

    // Select OpenGL (Core) as the desired graphics API
    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
    {
        TRACE("Could not set API! error: %d", eglGetError());
        goto _fail1;
    }

    // Get an appropriate EGL framebuffer configuration
    EGLConfig config;
    EGLint numConfigs;
    static const EGLint framebufferAttributeList[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE,     8,
        EGL_GREEN_SIZE,   8,
        EGL_BLUE_SIZE,    8,
        EGL_ALPHA_SIZE,   8,
        EGL_DEPTH_SIZE,   24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    eglChooseConfig(s_display, framebufferAttributeList, &config, 1, &numConfigs);
    if (numConfigs == 0)
    {
        TRACE("No config found! error: %d", eglGetError());
        goto _fail1;
    }

    // Create an EGL window surface
    s_surface = eglCreateWindowSurface(s_display, config, win, nullptr);
    if (!s_surface)
    {
        TRACE("Surface creation failed! error: %d", eglGetError());
        goto _fail1;
    }

    // Create an EGL rendering context
    static const EGLint contextAttributeList[] =
    {
        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
        EGL_CONTEXT_MAJOR_VERSION_KHR, 4,
        EGL_CONTEXT_MINOR_VERSION_KHR, 3,
        EGL_NONE
    };
    s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, contextAttributeList);
    if (!s_context)
    {
        TRACE("Context creation failed! error: %d", eglGetError());
        goto _fail2;
    }

    // Connect the context to the surface
    eglMakeCurrent(s_display, s_surface, s_surface, s_context);
    return true;

_fail2:
    eglDestroySurface(s_display, s_surface);
    s_surface = nullptr;
_fail1:
    eglTerminate(s_display);
    s_display = nullptr;
_fail0:
    return false;
}

static void deinitEgl()
{
    if (s_display)
    {
        eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (s_context)
        {
            eglDestroyContext(s_display, s_context);
            s_context = nullptr;
        }
        if (s_surface)
        {
            eglDestroySurface(s_display, s_surface);
            s_surface = nullptr;
        }
        eglTerminate(s_display);
        s_display = nullptr;
    }
}

static void setMesaConfig()
{
    // Uncomment below to disable error checking and save CPU time (useful for production):
    //setenv("MESA_NO_ERROR", "1", 1);

    // Uncomment below to enable Mesa logging:
    setenv("EGL_LOG_LEVEL", "debug", 1);
    setenv("MESA_VERBOSE", "all", 1);
    setenv("NOUVEAU_MESA_DEBUG", "1", 1);

    // Uncomment below to enable shader debugging in Nouveau:
    setenv("NV50_PROG_OPTIMIZE", "0", 1);
    setenv("NV50_PROG_DEBUG", "1", 1);
    setenv("NV50_PROG_CHIPSET", "0x120", 1);
}

// shader version retour d'info vecteur normal
#ifdef feedback_visibilite
static const char* const vertexShaderSource = R"text(
    #version 320 es

    layout (location = 0) in vec3 inPos;  // coordonnees points
    layout (location = 1) in vec2 inTexCoord;  // coordonnes dans la texture 
    layout (location = 2) in vec3 inNormal; // le vecteur normal du triangle

    out vec2 vtxTexCoord;
    out vec4 vtxNormalQuat;
    out vec3 vtxView;
	out float edz;
	
    uniform mat4 mdlvMtx;
    uniform mat4 projMtx;

    // Rotate the vector v by the quaternion q
    vec3 quatrotate(vec4 q, vec3 v)
    {
        return v + 2.0*cross(q.xyz, cross(q.xyz, v) + q.w*v);
    }

    void main()
    {
		// Calculate position
        vec4 pos = mdlvMtx * vec4(inPos, 1.0);
        vtxView = -pos.xyz;
        gl_Position = projMtx * pos;

        // Calculate normalquat
        vec3 normal = normalize(mat3(mdlvMtx) * inNormal);
        float z = (1.0 + normal.z) / 2.0;
        vtxNormalQuat = vec4(1.0, 0.0, 0.0, 0.0);
        if (z > 0.0)
        {
            vtxNormalQuat.z = sqrt(z);
            vtxNormalQuat.xy = normal.xy / (2.0 * vtxNormalQuat.z);
        }

        // Calculate texcoord
        vtxTexCoord = inTexCoord;
		
		vec3 viewVec = normalize(vtxView);
		vec4 normquat = normalize(vtxNormalQuat);
        vec3 normal2 = quatrotate(normquat, vec3(0.0, 0.0, 1.0));
		edz = max(dot(normal2, viewVec), 0.0);
		
		// visibilte = 
		
    }
)text";
#else
static const char* const vertexShaderSource = R"text(
    #version 320 es
	precision mediump float;

    layout (location = 0) in vec3 inPos;  // coordonnees points
    layout (location = 1) in vec2 inTexCoord;  // coordonnes dans la texture 
    layout (location = 2) in vec3 inNormal; // le vecteur normal du triangle

    out vec2 vtxTexCoord;
    out vec4 vtxNormalQuat;
    out vec3 vtxView;
	out float edz;
	
    uniform mat4 mdlvMtx;
    uniform mat4 projMtx;

    // Rotate the vector v by the quaternion q
    vec3 quatrotate(vec4 q, vec3 v)
    {
        return v + 2.0*cross(q.xyz, cross(q.xyz, v) + q.w*v);
    }

    void main()
    {
		// Calculate position
        vec4 pos = mdlvMtx * vec4(inPos, 1.0);
        vtxView = -pos.xyz;
        gl_Position = projMtx * pos;

        // Calculate normalquat
        vec3 normal = normalize(mat3(mdlvMtx) * inNormal);
        float z = (1.0 + normal.z) / 2.0;
        vtxNormalQuat = vec4(1.0, 0.0, 0.0, 0.0);
        if (z > 0.0)
        {
            vtxNormalQuat.z = sqrt(z);
            vtxNormalQuat.xy = normal.xy / (2.0 * vtxNormalQuat.z);
        }

        // Calculate texcoord
        vtxTexCoord = inTexCoord;
		
		
    }
)text";

#endif
	

static const char* const fragmentShaderSource = R"text(
    #version 320 es
	precision mediump float;

    in vec2 vtxTexCoord;
    in vec4 vtxNormalQuat;
    in vec3 vtxView;
	
	in float edz;
	
    out vec4 fragColor;
	
    uniform vec4 lightPos;
    uniform vec3 ambient;
    uniform vec3 diffuse;
    uniform vec4 specular; // w component is shininess

    uniform sampler2D tex_diffuse0;
		

    // Rotate the vector v by the quaternion q
    vec3 quatrotate(vec4 q, vec3 v)
    {
        return v + 2.0*cross(q.xyz, cross(q.xyz, v) + q.w*v);
    }

    void main()
    {
	    // Extract normal from quaternion
        vec4 normquat = normalize(vtxNormalQuat);
        vec3 normal = quatrotate(normquat, vec3(0.0, 0.0, 1.0));

        vec3 lightVec;
        if (lightPos.w != 0.0)
            lightVec = normalize(lightPos.xyz + vtxView);
        else
            lightVec = normalize(lightPos.xyz);

        vec3 viewVec = normalize(vtxView);
        vec3 halfVec = normalize(viewVec + lightVec);
		 
        float diffuseFactor = max(dot(lightVec, normal), 0.0);
		
        float specularFactor = pow(max(dot(normal, halfVec), 0.0), specular.w);

		//fragColor = texture(tex_diffuse0, vtxTexCoord);

		vec4 texDiffuseColor = texture(tex_diffuse0, vtxTexCoord);
		
        vec3 fragLightColor = ambient + diffuseFactor*diffuse*texDiffuseColor.rgb + specularFactor*specular.xyz;

        fragColor = vec4(min(fragLightColor, 1.0), texDiffuseColor.a);
    }
)text";



static GLuint s_program;
static GLuint s_vao, s_vbo;
static GLuint s_tex;

#ifdef feedback_visibilite
GLuint tbo;
#endif


#ifdef feedback_visibilite
const GLchar* feedbackVaryings[] = { "edz" };
#endif

static void sceneInit()
{
    GLint vsh = createAndCompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLint fsh = createAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    s_program = glCreateProgram();
    glAttachShader(s_program, vsh);
    glAttachShader(s_program, fsh);
	
	#ifdef feedback_visibilite
	// avant de linker on met en place la recuperation de la normale
	glTransformFeedbackVaryings(s_program, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
	#endif
	
    glLinkProgram(s_program);

    GLint success;
    glGetProgramiv(s_program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        char buf[512];
        glGetProgramInfoLog(s_program, sizeof(buf), nullptr, buf);
        TRACE("Link error: %s", buf);
    }
    glDeleteShader(vsh);
    glDeleteShader(fsh);

	


    loc_mdlvMtx = glGetUniformLocation(s_program, "mdlvMtx");
    loc_projMtx = glGetUniformLocation(s_program, "projMtx");
    loc_lightPos = glGetUniformLocation(s_program, "lightPos");
    loc_ambient = glGetUniformLocation(s_program, "ambient");
    loc_diffuse = glGetUniformLocation(s_program, "diffuse");
    loc_specular = glGetUniformLocation(s_program, "specular");
    loc_tex_diffuse0 = glGetUniformLocation(s_program, "tex_diffuse0");

	#ifdef feedback_visibilite
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, 1000, nullptr, GL_STATIC_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
	#endif

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);  
	glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);

    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(s_vao);

    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // Textures
    glGenTextures(1, &s_tex);
    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, s_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_texture_largeur, buffer_texture_hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
	
	
    
    // Uniforms
    glUseProgram(s_program);
    auto projMtx = glm::perspective(40.0f*TAU/360.0f, 1280.0f/720.0f, 0.01f, 1000.0f);
	
    glUniformMatrix4fv(loc_projMtx, 1, GL_FALSE, glm::value_ptr(projMtx));

		// vec4
    glUniform4f(loc_lightPos, 0.5f, 0.5f, 0.5f, 1.0f);
	
	// vec3
    glUniform3f(loc_ambient, 0.1f, 0.1f, 0.1f);
	// intensité couleurs de la texture
    glUniform3f(loc_diffuse, 1.0f, 1.0f, 1.0f);
	// intensité de la lumiere refletee
    glUniform4f(loc_specular, 0.1f, 0.1f, 0.1f, 20.0f);
    glUniform1i(loc_tex_diffuse0, 0); // texunit 0
	glUniform1i(loc_tex_diffuse1, 1); // texunit 0
	
    s_startTicks = armGetSystemTick();
}

//static float getTime()
//{
//    u64 elapsed = armGetSystemTick() - s_startTicks;
//    return (elapsed * 625 / 12) / 1000000000.0;
//}

void remplissage_module(void *args)
{
	
	    // Make sure the sample buffer size is aligned to 0x1000 bytes.
    u32 data_size = (SAMPLE_PER_BUF * CHANNELCOUNT * BYTESPERSAMPLE) ;

    u32 buffer_size = (data_size + 0xfff) & ~0xfff;

    // Allocate the buffer.
    void* out_buf_data1 = memalign(0x1000, buffer_size);
	void* out_buf_data2 = memalign(0x1000, buffer_size);
	
	memset(out_buf_data1, 0, buffer_size);
	memset(out_buf_data2, 0, buffer_size);
	
	u8* buffer_audio_temp = (u8*) malloc(SAMPLE_PER_BUFYM * CHANNELCOUNTYM * BYTESPERSAMPLEYM);

    // Initialize the default audio output device.
    rc = audoutInitialize();

	pointeur_buffer1 = &audout_buf1;
	// Prepare the audio data source buffer.
	pointeur_buffer1->next = NULL;
	pointeur_buffer1->buffer = out_buf_data1;
	pointeur_buffer1->buffer_size = buffer_size;
	pointeur_buffer1->data_size = data_size;
	pointeur_buffer1->data_offset = 0;
	
	pointeur_buffer2 = &audout_buf2;
	// Prepare the audio data source buffer.
	pointeur_buffer2->next = NULL;
	pointeur_buffer2->buffer = out_buf_data2;
	pointeur_buffer2->buffer_size = buffer_size;
	pointeur_buffer2->data_size = data_size;
	pointeur_buffer2->data_offset = 0;
	

	modctx_enigma = (modcontext*) malloc(sizeof(modcontext));
	modctx_ronken = (modcontext*) malloc(sizeof(modcontext));
	modctx_knulla = (modcontext*) malloc(sizeof(modcontext));
	bool res;
	res = hxcmod_init( modctx_enigma );
	res = hxcmod_init( modctx_ronken );
	res = hxcmod_init( modctx_knulla );


	// 48 KHZ, 0=pas de stereo_separation, 0 = pas de filter
	hxcmod_setcfg( modctx_enigma, SAMPLERATE, 0, 0);
	hxcmod_setcfg( modctx_ronken, SAMPLERATE, 0, 0);
	hxcmod_setcfg( modctx_knulla, SAMPLERATE, 0, 0);

	res = hxcmod_load( modctx_enigma, (void*)&enigma_mod_bin, enigma_mod_bin_size );
	res = hxcmod_load( modctx_ronken, (void*)&ronken_mod_bin, ronken_mod_bin_size );
	res = hxcmod_load( modctx_knulla, (void*)&knulla_mod_bin, knulla_mod_bin_size );
	

	hxcmod_fillbuffer(modctx_enigma,  (msample*) pointeur_buffer1->buffer, SAMPLE_PER_BUF , 0 );
	rc = audoutAppendAudioOutBuffer(pointeur_buffer1);
	hxcmod_fillbuffer(modctx_enigma,  (msample*) pointeur_buffer2->buffer, SAMPLE_PER_BUF , 0 );
	rc = audoutAppendAudioOutBuffer(pointeur_buffer2);

	// init YM beast
	pointeur_module_YM_beast = (unsigned char*) &beast_ym_bin;
	taille_module_YM_beast = beast_ym_bin_size;
	pMusic_beast = ymMusicCreate();
	ymMusicLoadMemory(pMusic_beast, pointeur_module_YM_beast  ,taille_module_YM_beast );
	ymMusicSetLoopMode(pMusic_beast,YMTRUE);
	ymMusicPlay(pMusic_beast);

	// init YM spritesTCB
	pointeur_module_YM_spritesTCB = (unsigned char*) &tcbsprites_ym_bin;
	taille_module_YM_spritesTCB = tcbsprites_ym_bin_size;
	pMusic_spritesTCB = ymMusicCreate();
	ymMusicLoadMemory(pMusic_spritesTCB, pointeur_module_YM_spritesTCB  ,taille_module_YM_spritesTCB );
	ymMusicSetLoopMode(pMusic_spritesTCB,YMTRUE);
	ymMusicPlay(pMusic_spritesTCB);

	// init YM sample TCB
	pointeur_module_YM_TCBsample = (unsigned char*) &tcbfull_ym_bin;
	taille_module_YM_TCBsample = tcbfull_ym_bin_size;
	pMusic_TCBsample = ymMusicCreate();
	ymMusicLoadMemory(pMusic_TCBsample, pointeur_module_YM_TCBsample  ,taille_module_YM_TCBsample );
	ymMusicSetLoopMode(pMusic_TCBsample,YMTRUE);
	ymMusicPlay(pMusic_TCBsample);

	// init YM sample ULM
	pointeur_module_YM_ULM = (unsigned char*) &ULMmegascroll_ym_bin;
	taille_module_YM_ULM = ULMmegascroll_ym_bin_size;
	pMusic_ULM = ymMusicCreate();
	ymMusicLoadMemory(pMusic_ULM, pointeur_module_YM_ULM  ,taille_module_YM_ULM );
	ymMusicSetLoopMode(pMusic_ULM,YMTRUE);
	ymMusicPlay(pMusic_ULM);	



    // Start audio playback.
    rc = audoutStartAudioOut();
	
	
	while(running)
	{
		rc = audoutGetReleasedAudioOutBuffer(&audout_released_buf, &nbbufrelease);
		if (nbbufrelease == 1)
				{
					if (numero_mod == 0 ) hxcmod_fillbuffer(modctx_enigma,  (msample*) audout_released_buf->buffer, SAMPLE_PER_BUF , 0 );
					if (numero_mod == 4 ) hxcmod_fillbuffer(modctx_ronken,  (msample*) audout_released_buf->buffer, SAMPLE_PER_BUF , 0 );
					if (numero_mod == 6 ) hxcmod_fillbuffer(modctx_knulla,  (msample*) audout_released_buf->buffer, SAMPLE_PER_BUF , 0 );
					
					if (numero_mod == 1 ) ymMusicCompute(pMusic_TCBsample, (ymsample*) buffer_audio_temp ,SAMPLE_PER_BUFYM);
					if (numero_mod == 2 ) ymMusicCompute(pMusic_spritesTCB, (ymsample*) buffer_audio_temp  ,SAMPLE_PER_BUFYM);
					if (numero_mod == 3 ) ymMusicCompute(pMusic_ULM, (ymsample*) buffer_audio_temp  ,SAMPLE_PER_BUFYM);
					if (numero_mod == 5 ) ymMusicCompute(pMusic_beast, (ymsample*) buffer_audio_temp  ,SAMPLE_PER_BUFYM);
					
					if (numero_mod == 1 or numero_mod == 2 or numero_mod == 3 or numero_mod == 5)
					{
						// recopie basic, 16 bits par 16 bits
						u16* pointeur_source_audioYM = (u16*) buffer_audio_temp;
						u16* pointeur_dest_audioYM = (u16*) audout_released_buf->buffer;
			
						int post_dest_audioYM=0;
						int pos_source_audioYM = 0;
			
						float pos_en_cours_source_a_virgule=0;
						float increment_source = (float) ((float) SAMPLERATEYM / (float) SAMPLERATE);
						// 44100 / 48000 = inferieur a 1
			
						for (int i=0;i<SAMPLE_PER_BUF;i++)
						{
							pos_source_audioYM = (int) pos_en_cours_source_a_virgule;
							u16 sample=pointeur_source_audioYM[pos_source_audioYM];
						
							// sample = sample / diviseur_volume_global;
							pos_en_cours_source_a_virgule = pos_en_cours_source_a_virgule + increment_source;
				
							// a gauche
							pointeur_dest_audioYM[post_dest_audioYM] = sample;
							post_dest_audioYM++;
							// a droite
							pointeur_dest_audioYM[post_dest_audioYM] = sample;
							post_dest_audioYM++;
						}
					}
					
					
					rc = audoutAppendAudioOutBuffer(audout_released_buf);
				}
		svcSleepThread(7000000); 
		// TRACE("face_la_plus_visible CPU2 : %d\n",face_la_plus_visible);
		audrenWaitFrame();
	}
}


//static int facevisible(float vecteur_normal_x,float vecteur_normal_y,float vecteur_normal_z, float x_point, float y_point, float z_point)
static int facevisible(int numero_point_calcul, int numero_de_face)

{
		int result=0;
	
		// backface culling
		// Le principe est simple : on calcule d'une part le vecteur entre la position de la caméra et le centre du polygone à tester, et d'autre part le vecteur normal à ce même polygone. 
		// Si leur produit scalaire est positif alors cela signifie que le triangle est orienté dans le même sens que la vue (il nous tourne le dos), et qu'il peut donc être éliminé du rendu. 
		// position de la caméré : 0,0,-2
		// 
		//
		// en entrée : vecteur normal
		// en entrée : 1 point
		// rotation du vecteur normal
		// calcul du vecteur de point à observateur en (0,0,-2)
		// rotation du vecteur normal
		// rotation du vecteur du point 
		// dot 
		/*
		glm::mat4 matrice_normale{1.0};
		vecteur_normal_x = vecteur_normal_x - x_point;
		vecteur_normal_y = vecteur_normal_y - y_point;
		vecteur_normal_z = vecteur_normal_z - z_point;
		
		matrice_normale = glm::translate(matrice_normale, glm::vec3{vecteur_normal_x, vecteur_normal_y, vecteur_normal_z});
		matrice_normale = glm::rotate(matrice_normale, pos_rotation_x, glm::vec3{1.0f, 0.0f, 0.0f});
		matrice_normale = glm::rotate(matrice_normale, pos_rotation_y, glm::vec3{0.0f, 1.0f, 0.0f});
		matrice_normale = glm::rotate(matrice_normale, pos_rotation_z, glm::vec3{0.0f, 0.0f, 1.0f});
    	// matrice normale rotatée
		
		glm::mat4 matrice_point{1.0};
		// calcul vecteur de la camera au point du triangle
		float cameratotriangle_x = x_point - 0 ;
		float cameratotriangle_y = y_point - 0;
		float cameratotriangle_z = z_point - (-2);
		
		matrice_point = glm::translate(matrice_point, glm::vec3{cameratotriangle_x, cameratotriangle_y, cameratotriangle_z});
		matrice_point = glm::rotate(matrice_point, pos_rotation_x, glm::vec3{1.0f, 0.0f, 0.0f});
		matrice_point = glm::rotate(matrice_point, pos_rotation_y, glm::vec3{0.0f, 1.0f, 0.0f});
		matrice_point = glm::rotate(matrice_point, pos_rotation_z, glm::vec3{0.0f, 0.0f, 1.0f});
		// point rotaté
		
		glm::vec3 matrice_normale_vec3;
		matrice_normale_vec3 = glm::vec3(matrice_normale[3]);
		matrice_normale_vec3 = glm::normalize(matrice_normale_vec3);
		glm::vec3 matrice_point_vec3;
		matrice_point_vec3 = glm::vec3(matrice_point[3]);
		matrice_point_vec3 = glm::normalize(matrice_point_vec3);
		
		float result_dot;
		result_dot = glm::dot( matrice_normale_vec3, matrice_point_vec3);
		
		
		if (result_dot<= 0) result=1;
		
		
		
		
		
		// rotation du point de l'objet
		float new_z_point = (( valeur_C * x_point) + (valeur_F * y_point ) + ( valeur_I * z_point));
		float new_x_point = (( valeur_A * x_point) + ( valeur_D * y_point ) + (valeur_G * z_point));
		float new_y_point = (( valeur_B * x_point) + (valeur_E * y_point) + (valeur_H * z_point));
		
		// rotation du vecteur normal
		float new_vecteur_normal_z = (( valeur_C * vecteur_normal_x) + (valeur_F * vecteur_normal_y ) + ( valeur_I * vecteur_normal_z));
		float new_vecteur_normal_x = (( valeur_A * vecteur_normal_x) + ( valeur_D * vecteur_normal_y ) + (valeur_G * vecteur_normal_z));
		float new_vecteur_normal_y = (( valeur_B * vecteur_normal_x) + (valeur_E * vecteur_normal_y ) + (valeur_H * vecteur_normal_z));
		
		
		
		new_vecteur_normal_x = new_vecteur_normal_x - new_x_point;
		new_vecteur_normal_y = new_vecteur_normal_y - new_y_point;
		new_vecteur_normal_z = new_vecteur_normal_z - new_z_point;
		
		// norme d'un vecteur : √(x² + y² + z²)
		float normalisation_vecteur_normal = sqrtf ( (new_vecteur_normal_x * new_vecteur_normal_x) + (new_vecteur_normal_y * new_vecteur_normal_y) + (new_vecteur_normal_z * new_vecteur_normal_z));
		new_vecteur_normal_x = new_vecteur_normal_x / normalisation_vecteur_normal;
		new_vecteur_normal_y = new_vecteur_normal_y / normalisation_vecteur_normal;
		new_vecteur_normal_z = new_vecteur_normal_z / normalisation_vecteur_normal;
		
		TRACE("new_vecteur_normal_x : %f\n",new_vecteur_normal_x);
		TRACE("new_vecteur_normal_y : %f\n",new_vecteur_normal_y);
		TRACE("new_vecteur_normal_z : %f\n",new_vecteur_normal_z);

		
		// vecteur dirigé du point calculé à 0,0,+1
		// par exemple de 0,0,1 à 0,0,-1 : 0,0,-2

		
		TRACE("new_X_point : %f\n",new_x_point);
		TRACE("new_Y_point : %f\n",new_y_point);
		TRACE("new_Z_point : %f\n",new_z_point);
		
		// de la caméra au triangle
		new_x_point = new_x_point-0;
		new_y_point = new_y_point-0;
		new_z_point = new_z_point-( 2);
		

		TRACE("new_X_point vecteur observateur : %f\n",new_x_point);
		TRACE("new_Y_point vecteur observateur : %f\n",new_y_point);
		TRACE("new_Z_point vecteur observateur : %f\n",new_z_point);		
		
		float normalisation_vecteur_point = sqrtf ( (new_x_point * new_x_point) + (new_y_point * new_y_point) + (new_z_point * new_z_point));
		TRACE("normalisation_vecteur_point : %f\n",normalisation_vecteur_point);
		new_x_point = new_x_point / normalisation_vecteur_point;
		new_y_point = new_y_point / normalisation_vecteur_point;
		new_z_point = new_z_point / normalisation_vecteur_point;


		
		TRACE("vecteur du point a l observateur X normalise: %f\n",new_x_point);
		TRACE("vecteur du point a l observateur Y normalise: %f\n",new_y_point);
		TRACE("vecteur du point a l observateur Z normalise: %f\n",new_z_point);



		// float scalaire = new_vecteur_normal_z * -1;
		float scalaire = (new_vecteur_normal_x*new_x_point)+(new_vecteur_normal_y*new_y_point)+(new_vecteur_normal_z*new_z_point);
		
		TRACE("scalaire  : %f\n",scalaire);
		
		if (scalaire<= 0) result=1;
		
		TRACE("result : %d\n",result);

		*/
		
	// TRACE("normal vertex 1 Z : %f\n",vertex_list[numero_point_calcul].normal[2]);
	// vec3 inPos;
	glm::vec3 inPos_vec3 = glm::make_vec3(vertex_list[numero_point_calcul].position);
	// vec3 inNormal
	glm::vec3 inNormal = glm::make_vec3(vertex_list[numero_point_calcul].normal);
	// TRACE("normal vertex 0 Z en vec3: %f\n",inNormal[2]);
	glm::mat3 mdlvMtx_mat3;
	mdlvMtx_mat3 = glm::mat3(mdlvMtx);
	// vec3 normal = normalize(mat3(mdlvMtx) * inNormal);
	glm::vec3 normal = mdlvMtx_mat3 * inNormal;
	normal = glm::normalize(normal);
	//float z = (1.0 + normal.z) / 2.0;
	float zshader = normal.z;
	zshader = (1.0 + zshader) / 2.0;
	// vec4 pos = mdlvMtx * vec4(inPos, 1.0);
	glm::vec4 pos_vec4;
	pos_vec4 = mdlvMtx * glm::vec4(inPos_vec3, 1.0);
	// vtxView = -pos.xyz;
	glm::vec3 vtxView;
	vtxView = -(pos_vec4.xyz());
	glm::vec3 viewVec = glm::normalize(vtxView);
	// vtxNormalQuat = vec4(1.0, 0.0, 0.0, 0.0);
	glm::vec4 vtxNormalQuat = glm::vec4(1.0, 0.0, 0.0, 0.0);
	if (zshader > 0.0)
        {
			// vtxNormalQuat.z = sqrt(z);
            vtxNormalQuat.z = sqrt(zshader);
			// vtxNormalQuat.xy = normal.xy / (2.0 * vtxNormalQuat.z);
            vtxNormalQuat.x = normal.x / (2.0 * vtxNormalQuat.z);
			vtxNormalQuat.y = normal.y / (2.0 * vtxNormalQuat.z);
        }
	// vec4 normquat = normalize(vtxNormalQuat);
	glm::vec4 normquat = glm::normalize(vtxNormalQuat);
	// vec3 normal2 = quatrotate(normquat, vec3(0.0, 0.0, 1.0));
	// vec3 quatrotate(vec4 q, vec3 v)
	// {
	// return v + 2.0*cross(q.xyz, cross(q.xyz, v) + q.w*v);
	// toto1 = q.xyz
	// toto2 = cross(toto1, v)
	// toto3 = toto2 + q.w*v
	// toto4 = q.xyz
	// v + 2.0*cross(toto4, toto3);
	// glm::vec3 normal2;
	glm::vec3 quatrotate_v = glm::vec3(0.0, 0.0, 1.0);
	glm::vec3 toto1 = normquat.xyz();
	glm::vec3 toto2 = glm::cross(toto1, quatrotate_v);
	glm::vec3 toto3 = normquat.w * quatrotate_v;
	toto3 = toto2 + toto3;
	glm::vec3 toto4 = normquat.xyz();
	glm::vec3 resultat_quatrotate;
	resultat_quatrotate = glm::cross(toto4, toto3);
	resultat_quatrotate =  resultat_quatrotate * 2.0f;
	resultat_quatrotate =  quatrotate_v + resultat_quatrotate;
	float edz = glm::dot(resultat_quatrotate, viewVec);
	edz = glm::max(edz, 0.0f);
	//TRACE("EDZ calculé pourri 15 2,4: %f\n",edz);
	//normal2 = quatrotate_v + 2.0*cross(normquat.xyz(), cross(normquat.xyz(), quatrotate_v) + normquat.w*quatrotate_v);
		
	if (edz > 0.0) 
	{
		result = 1;
		if (normal_max<edz)
		{
			normal_max = edz;
			face_la_plus_visible = numero_de_face;
		}
	}
	return result;
		
		
		
}

static void sceneUpdate()
{
	// icila
	// face 0 : boink pos_rotation_z=(-MONPI);
	// face 1 : fullscreen TCB : pos_rotation_x=MONPI*1; + pos_rotation_z=(-MONPI/2);
	// face 2 : sprites TCB : pos_rotation_x=MONPI*1.5; pos_rotation_y=(MONPI*1); pos_rotation_z=(-MONPI/2);
	// face 3 : megascroll ULM : pos_rotation_x=MONPI*0.5; pos_rotation_y=(MONPI*0.5);pos_rotation_z=(-MONPI/2);
	// face 4 : TechTech : pos_rotation_y=(MONPI*1.5); + pos_rotation_z=(-MONPI/2);
	// face 5 : Shadow of the Beast : pos_rotation_x=MONPI*1.5;pos_rotation_y=(MONPI*1); pos_rotation_z=0

	// intensité couleurs de la texture
    glUniform3f(loc_diffuse, luminosite, luminosite, luminosite);	
	
	//pos_rotation_x=(MONPI*1.5)-(0.015*45);
	//pos_rotation_x=MONPI*1.5;
	//pos_rotation_x=MONPI*.2; // face 6 devant avec ce x
	 // // pos_rotation_x=MONPI*1; // face 6 derriere avec ce x
	pos_rotation_x=pos_rotation_x+increment_pos_rotation_x;
	if (pos_rotation_x >= (MONPI*2)) pos_rotation_x=0.0f;

	//pos_rotation_y=(MONPI*1);
	//pos_rotation_y=(MONPI*1.5);
	pos_rotation_y=pos_rotation_y+increment_pos_rotation_y;
	if (pos_rotation_y >= (MONPI*2)) pos_rotation_y=0.0f;
	
	//pos_rotation_z=(-MONPI/2);
	pos_rotation_z=pos_rotation_z+increment_pos_rotation_z;
	if (pos_rotation_z >= (MONPI*2)) pos_rotation_z=0.0f;
	
	glm::mat4 mdlvMtx2{1.0};
    // mdlvMtx = mdlvMtx2;
    // zoom
	// ici x et Y et Z pour deplacer
	mdlvMtx = glm::translate(mdlvMtx2, glm::vec3{pos_x_cube_opengl, pos_y_cube_opengl, pos_z_cube_opengl});
	
	
	mdlvMtx = glm::rotate(mdlvMtx, pos_rotation_x, glm::vec3{1.0f, 0.0f, 0.0f});
	mdlvMtx = glm::rotate(mdlvMtx, pos_rotation_y, glm::vec3{0.0f, 1.0f, 0.0f});
    mdlvMtx = glm::rotate(mdlvMtx, pos_rotation_z, glm::vec3{0.0f, 0.0f, 1.0f});
    
	// 
    glUniformMatrix4fv(loc_mdlvMtx, 1, GL_FALSE, glm::value_ptr(mdlvMtx));

	if (mode_manuel==0)
	{
		duree_animation_encours--;
		if (duree_animation_encours <0)
		{
			// prochaine scene d'animation
			//TRACE("changement  animation");
			numero_animation_en_cours++;
			if (numero_animation_en_cours < nombre_total_animation)
			{
				//TRACE("changement reel animation");
				if ( liste_animation[numero_animation_en_cours].angle_x_initial != -1) pos_rotation_x = liste_animation[numero_animation_en_cours].angle_x_initial;
				if ( liste_animation[numero_animation_en_cours].angle_y_initial != -1)  pos_rotation_y = liste_animation[numero_animation_en_cours].angle_y_initial;
				if ( liste_animation[numero_animation_en_cours].angle_z_initial != -1)  pos_rotation_z = liste_animation[numero_animation_en_cours].angle_z_initial;
				increment_pos_rotation_x = liste_animation[numero_animation_en_cours].increment_angle_x;
				increment_pos_rotation_y = liste_animation[numero_animation_en_cours].increment_angle_y;
				increment_pos_rotation_z = liste_animation[numero_animation_en_cours].increment_angle_z;
				duree_animation_encours = liste_animation[numero_animation_en_cours].duree_animation;
				pos_x_cube_opengl = liste_animation[numero_animation_en_cours].posx_initial;
				pos_y_cube_opengl = liste_animation[numero_animation_en_cours].posy_initial;
				pos_z_cube_opengl = liste_animation[numero_animation_en_cours].posz_initial;
			}
		}
	}
}



static void CLSRender()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void sceneRender()
{
	
    //glClearColor(0x68/255.0f, 0xB0/255.0f, 0xD8/255.0f, 1.0f);
	
	
   	currentTick = armGetSystemTick();
	diffrenceTick = currentTick-previousTick;
	differenceNS = armTicksToNs(diffrenceTick);
	toto = (float) differenceNS;
	pourcentageavantdraw = (toto / TICKS_PER_FRAME)*100;
	
    


	
    // draw our textured cube

	#ifdef feedback_visibilite
	//glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
    
	glBeginTransformFeedback(GL_TRIANGLES);
	#endif
	



	// glDrawArrays(GL_TRIANGLES, 0, vertex_list_count);
	
	// face 0
    if (face0visible == 1) glDrawArrays(GL_TRIANGLES, 0, 6);
	// face 1
	if (face1visible == 1) glDrawArrays(GL_TRIANGLES, 6, 6);
	// face 2
	if (face2visible == 1) glDrawArrays(GL_TRIANGLES, 12, 6);
	// face 3
	if (face3visible == 1) glDrawArrays(GL_TRIANGLES, 18, 6);
	// face 4
	if (face4visible == 1) glDrawArrays(GL_TRIANGLES, 24, 6);
	// face 5
	if (face5visible == 1) glDrawArrays(GL_TRIANGLES, 30, 6);


	#ifdef feedback_visibilite
	glEndTransformFeedback();
	glFlush();
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);
	#endif

		
		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageapresdraw = (toto / TICKS_PER_FRAME)*100;
	
	
	//for(int i=0;i<6;i++)
	//{
	//	int ii=i*6;
	//	TRACE("feedback %i / 1: %f 2: %f 3:%f 4: %f 5: %f 6: %f \n", i,feedback[0+ii], feedback[1+ii], feedback[2+ii], feedback[3+ii], feedback[4+ii], feedback[5+ii]);
	//}
	
	
    
	
	
}

static void sceneExit()
{
    glDeleteTextures(1, &s_tex);
	glDeleteBuffers(1, &s_vbo);
    glDeleteVertexArrays(1, &s_vao);
    glDeleteProgram(s_program);
}


// precalcule les cos et sin
void	precalc_sin_cos()
{
	float	anglecourant;
	for(int i=0;i<3600;i++)
	{
		anglecourant = (i * ( MONPI / 1800));
		tableau_sin[i] = sin(anglecourant);
		tableau_cos[i] = cos(anglecourant);
	}
}

void	calc_valeurs_globales(int anglex_a,int angley_b,int anglez_c)
{
//	float cos_a,sin_a,cos_b,sin_b,cos_c,sin_c;
	cos_x = tableau_cos[anglex_a];
	sin_x = tableau_sin[anglex_a];
    cos_y = tableau_cos[angley_b];
    sin_y = tableau_sin[angley_b];
    cos_z = tableau_cos[anglez_c];
    sin_z = tableau_sin[anglez_c];

	valeur_A = cos_z * cos_y;
	valeur_B = sin_z * cos_y;
	valeur_C = -(sin_y);
	valeur_D = (cos_z * sin_y * sin_x) - (sin_z * cos_x);
	valeur_E = ( cos_z * cos_x ) + (sin_z * sin_y * sin_x);
	valeur_F = cos_y * sin_x;
	valeur_G = ( sin_z * sin_x ) + ( cos_z * sin_y * cos_x);
	valeur_H = (sin_z * sin_y * cos_x) - (cos_z * sin_x );
	valeur_I = cos_y*cos_x;


}

void	calc_point(int valeur_P,int nb_points, struct  struct_point *points)
{
	//int	x_finalo, y_finalo;
	//int     x_finald, y_finald;
	float	valeur_Q, new_x,new_y,new_z;

	for(int i=0;i<nb_points;i++)
		{

		new_z = (( valeur_C * points[i].x) + (valeur_F * points[i].y ) + ( valeur_I * points[i].z) + Zdep);
		valeur_Q = 1. - ( new_z / valeur_P);

		new_x = (( valeur_A * points[i].x) + ( valeur_D * points[i].y ) + (valeur_G * points[i].z) + Xdep);
		new_y = (( valeur_B * points[i].x) + (valeur_E * points[i].y ) + (valeur_H * points[i].z) + Ydep);

		points[i].xrotated = new_x;
		points[i].yrotated = new_y;
		points[i].zrotated = new_z;
		// points[i].valeur_Q = -new_z+valeur_P;
		points[i].valeur_Q = valeur_Q;
		points[i].x2D = (new_x / valeur_Q)+posx2D;
		points[i].y2D = (new_y / valeur_Q)+posy2D;
		}

	//valeur_Q = 1. - ((( valeur_C * droite_a_calculer->xdestination) + (valeur_F * droite_a_calculer->ydestination ) + ( valeur_I * droite_a_calculer->zdestination) + Zdep) / valeur_P);
        //x_finald = (( valeur_A * droite_a_calculer->xdestination) + ( valeur_D * droite_a_calculer->ydestination ) + (valeur_G * droite_a_calculer->zdestination) + Xdep)/ valeur_Q;
        //y_finald = (( valeur_B * droite_a_calculer->xdestination) + (valeur_E * droite_a_calculer->ydestination ) + (valeur_H * droite_a_calculer->zdestination) + Ydep) /valeur_Q;

	//draw_line(x_finalo, y_finalo,x_finald,y_finald, droite_a_calculer->couleur);

	return;
}

void	plotedz(float xf,float yf, u32 couleur)
{
	// TRACE("framebuf dans plotedz : %x\n",framebuf);
	
	int x,y;
	x=(int)xf;
	y=(int)yf;
	
	x=x + (boink_screen_largeur/2);
	y=y + (boink_screen_hauteur/2);

	if (x<0) 
		{ return;}
	if (y<0)
        { return;}

	if (x>boink_screen_largeur-1)
		{ return;}

	if (y>boink_screen_hauteur-1)
		{return;}

	int destinationbuf = x + ( buffer_texture_largeur *y);
	texture_screen_buffer[destinationbuf] = couleur;

	return;
}

void	efface_tous_les_ecrans()
{
	
	memset(texture_screen_buffer, 0, buffer_texture_largeur*buffer_texture_hateur_un_ecran*nb_ecran_actuel*4);
		
	
	return;

}

void	draw_ligne_n(int x1,int y1, int x2, int y2, u32 couleur)
{
	int	dx,dy, i,xinc,yinc,cumul,x,y;
	x = x1;
	y = y1;

	// dx toujours positif
	dx = x2 - x1;
	dy = y2 - y1;

	xinc = ( dx > 0 ) ? 1 : -1 ;
	yinc = ( dy > 0 ) ? 1 : -1 ;

	dx = abs(dx) ;
	dy = abs(dy) ;

	plotedz(x,y,couleur);

	// tester dx>dy ? pente ??
	if (dx>dy)
		{
		 cumul = dx / 2 ;
		 for ( i = 1 ; i <= dx ; i++ ) 
			{
			x += xinc ;
			cumul += dy ;
			if ( cumul >= dx ) {
				cumul -= dx ;
				y += yinc ; }
			plotedz(x,y,couleur);
			}
		}
	else
		{
		cumul = dy / 2 ;
		for ( i = 1 ; i <= dy ; i++ ) 
			{
			y += yinc ;
			cumul += dx ;
			if ( cumul >= dy ) {
				cumul -= dy ;
				x += xinc ; }
			plotedz(x,y,couleur);
			}
	
		}
}


// y2=y1
void    draw_ligne_horizontale(int x1,int y1, int x2, int y2, u32 couleur)
{
	if (x1>x2)
	{
		int t;
		t=x1;
		x1=x2;
		x2=t;
	}

	for(int i=x1;i<=x2;i++)
		{
		plotedz(i,y1,couleur);
		} 

}

// x2=x1
void	draw_ligne_verticale(int x1,int y1, int x2, int y2, u32 couleur)
{
	if (y1>y2)
        {
                int t;
                t=y1;
                y1=y2;
                y2=t;
        }

        for(int i=y1;i<=y2;i++)
                {
                plotedz(x1,i,couleur); 
                } 
}


void	draw_line(int x1,int y1, int x2, int y2, u32 couleur)
{
	// trace une ligne de x1,y1 a x2,y2

	// ligne horizontale ? 
	if (y1 == y2)
		{draw_ligne_horizontale(x1,y1, x2, y2, couleur);}

	if (x1 == x2)
		{draw_ligne_verticale(x1,y1, x2, y2,couleur);}

	if (x2 < x1)
		{draw_ligne_n(x2,y2,x1,y1,couleur);}
	else
		{ draw_ligne_n(x1,y1,x2,y2,couleur);}
}

void	tri_faces(struct  struct_objet *objet)
{
	// calcul Z moyen
	int	zmoyen;
	struct  struct_point *points;
	points = objet->liste_des_points;
	
	int nombre_de_faces = objet->nb_faces;
	int ci2;
	for(ci2=0;ci2 < nombre_de_faces; ci2++)
		{
			// c est un triangle forcement
			
			zmoyen = points[objet->liste_des_triangles[ci2].p1].zrotated;
			zmoyen = zmoyen + points[objet->liste_des_triangles[ci2].p2].zrotated;
			zmoyen = zmoyen + points[objet->liste_des_triangles[ci2].p3].zrotated;
			zmoyen = zmoyen / 3;
			faces_triees[ci2].Z_moyen_face = zmoyen;
			faces_triees[ci2].numero_face = ci2;
		}
	
	// tri
	int	triok;
	int	numerodelaface1;
	int	zmoyen1;
	do
	{
		triok = 1;
		for (int i=0;i<objet->nb_faces-2;i++)
			{ 

			if (faces_triees[i].Z_moyen_face < faces_triees[i+1].Z_moyen_face)
				{
				triok=0;
				numerodelaface1 = faces_triees[i].numero_face;
				zmoyen1 = faces_triees[i].Z_moyen_face;
				faces_triees[i].numero_face = faces_triees[i+1].numero_face;
				faces_triees[i].Z_moyen_face=faces_triees[i+1].Z_moyen_face;
				faces_triees[i+1].numero_face = numerodelaface1;
				faces_triees[i+1].Z_moyen_face = zmoyen1;
			
				}
			}
	} while (triok == 0);
	
	
}

void calc_ligne_cote(int xa, int ya,int xb,int yb)
{
	int* pointeur_sur_tableau_x;
	xag = xa;
	yag=ya;
	xbg=xb;
	ybg=yb;
	
	if (ya == yb)
			// ligne horizontale
		{
		ousuisje=2;
		// on ne fait rien
		}
	else
		{
			int	dx,dy;
			pointeur_sur_tableau_x = tableau_x_min;
			if (ya>yb)
				{
				ousuisje=22;
				// ligne montante => forcement sur la droite
				pointeur_sur_tableau_x = tableau_x_max;
				int xtemp, ytemp;
				xtemp = xa;
				xa = xb;
				xb=xtemp;
				ytemp = ya;
                ya = yb;
                yb=ytemp;
				xag = xa;
                yag=ya;
                xbg=xb;
                ybg=yb;
 				}
			dx = xb - xa;
			if (dx == 0)
				{
					ousuisje=23;
					// ligne verticale
					// clipping haut
					if (ya<0)
						{ya=0;}
					// la ligne est entiere au dessus de l'ecran
					if (yb>0)
					{
						ousuisje=24; 
						// clipping bas
						if (yb>(boink_screen_hauteur-1) )
							{yb=(boink_screen_hauteur-1) ;}
						// la ligne est entiere au dessous de l'ecran
						if (ya<boink_screen_hauteur )
							{ousuisje=25; 
							// test miny
							if (ya<y_min)
							{y_min=ya;}
							if (yb>y_max)
								{y_max=yb;}
				
							for (int i=ya;i<=yb;i++)
								{pointeur_sur_tableau_x[i]=xa;}
							}
					}
				}
			else
				{
					ousuisje=31;
					// ligne normale
					dy = yb-ya;
					float	pente;
					pente = (float)dx / dy;
					float	x_en_cours;
					// clipping haut
                    if (ya<0)
					{
						ousuisje=32;
						xa = xa-((float)(ya * (float)pente));
						ya = 0;
						dy = yb;
					}
					// la ligne est entiere au dessus de l'ecran
					if (yb>0)
						{
						ousuisje=5; 
						// clipping en bas
						if (yb>(boink_screen_hauteur -1))
							{
						ousuisje=33;
						yb=boink_screen_hauteur-1 ;
						dy = (boink_screen_hauteur -1) -ya;
						}
							// la ligne est entiere au dessous de l'ecran
							if (ya<boink_screen_hauteur )
                               	{ 
								ousuisje=34;
							if (yb > y_max)
                        	    {y_max = yb;}
							if (ya < y_min)
                            	{y_min = ya;}
							ousuisje=6;
							x_en_cours=xa;
							int index1 = ya;
							for (int i=index1;i<yb;i++)
                            	{
								pointeur_sur_tableau_x[i]=(int)x_en_cours;
								x_en_cours = x_en_cours + pente;
								index1++;
								}
							// derniere valeur
							pointeur_sur_tableau_x[index1]=(int)x_en_cours;
							}
						}
				}
			
        }
	
}

void draw_triangle(int pr1, int pr2, int pr3, u32 couleurface  )
{
	
    y_min=boink_screen_hauteur-1;
    y_max=0;

	// de pr1 à pr2
	int xa,ya,xb,yb;
	xa = (points[pr1].x2D)+(boink_screen_largeur/2);
	ya = (points[pr1].y2D)+(boink_screen_hauteur/2);
	xb = (points[pr2].x2D)+(boink_screen_largeur/2);
	yb = (points[pr2].y2D)+(boink_screen_hauteur/2);
	calc_ligne_cote(xa,ya,xb,yb);
	
	xa = (points[pr2].x2D)+(boink_screen_largeur/2);
	ya = (points[pr2].y2D)+(boink_screen_hauteur/2);
	xb = (points[pr3].x2D)+(boink_screen_largeur/2);
	yb = (points[pr3].y2D)+(boink_screen_hauteur/2);
	calc_ligne_cote(xa,ya,xb,yb);

	xa = (points[pr3].x2D)+(boink_screen_largeur/2);
	ya = (points[pr3].y2D)+(boink_screen_hauteur/2);
	xb = (points[pr1].x2D)+(boink_screen_largeur/2);
	yb = (points[pr1].y2D)+(boink_screen_hauteur/2);
	calc_ligne_cote(xa,ya,xb,yb);

	// remplissage
	int	xgauche,xdroit;
	int posy_fill=0;
	for(int i=y_min;i<=y_max;i++)
		{
			posy_fill = i*buffer_texture_largeur;
			xgauche = tableau_x_min[i];
			if (xgauche<0)
				{xgauche=0;}
			xdroit = tableau_x_max[i];
			if (xdroit>(boink_screen_largeur-1))
				{xdroit=boink_screen_largeur-1;}
			for(int j=xgauche;j<=xdroit;j++)
				{texture_screen_buffer[posy_fill+j] = couleurface;}
		}
	
}


// -------------------------------------
// routines sprites TCB
// -------------------------------------


static int  mod(int v,int m)
{ 
	while (v<0)
	{
		v += m;
	}
	return v % m; 
} 


static void cls_sprites_TCB()
{
	for (int i=0;i<largeur_sprites_TCB*hauteur_sprites_TCB;i++)
	{
		buffer_ecran[i] = fond_ecran[i];
	}
}

static void affiche_sprite_1616(int x_sprite, int y_sprite)
{
	int pos_source = 0;
	int pos_dest = (y_sprite * largeur_sprites_TCB) + x_sprite;
	
	for (int y=0;y<hauteur_ball_sprites_TCB;y++)
	{
		for (int x=0;x<largeur_ball_sprites_TCB;x++)
		{
			u32 pixel = ball[pos_source];
			if (pixel != 0)
			{
				buffer_ecran[pos_dest] = ball[pos_source];
			}
			pos_dest++;
			pos_source++;
		}
		pos_dest=pos_dest + largeur_sprites_TCB - largeur_ball_sprites_TCB;
	
	}

	
}

static void initcossin()
{
	float PILEN2 = PILEN;
	for (int i=0;i<PILEN;i++)
	{ 
		float a =  ((i * 2.0f * 3.1415926f) * (1.0f / PILEN2)); 
		//TRACE("a = %f\n",a);
	  	cosTab[i] = (int)(32767.f * (float) cos(a)); 
	  	sinTab[i] = (int)(32767.f * (float) sin(a)); 
	} 
	// TRACE("cosTab[1] : %f\n",cosTab[1]);
}

//-----------------------------------------------------------------------------
//               routines Shadow of the Beast 
//-----------------------------------------------------------------------------

static void affiche_sprite_Amiga_Shadow(int pos_x_dest_sprite_Amiga_Shadow , int largeur_sprite_Amiga_Shadow , int hauteur_sprite_Amiga_Shadow , int pos_x_source_sprite_Amiga_Shadow, int pos_y_source_sprite_Amiga_Shadow , int pos_y_dest_sprite_Amiga_Shadow  )
{
	//#define largeur_sprite_Amiga_Shadow 134
	//#define hauteur_sprite_Amiga_Shadow 139
	//#define pos_y_source_sprite_Amiga_Shadow 378
	//#define pos_y_dest_sprite_Amiga_Shadow 70

	
	

	u32 pixel;
	
	// completement a gauche, invisible
	if ((pos_x_dest_sprite_Amiga_Shadow + largeur_sprite_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow) < 0) return;
	
	// clipping gauche à 48 pixels
	if ((pos_x_dest_sprite_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow) <0 )
	{
		int pos_source = pos_y_source_sprite_Amiga_Shadow*largeur_png_sprites_Amiga_Shadow-(pos_x_dest_sprite_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow)+pos_x_source_sprite_Amiga_Shadow;
		int pos_dest = bord_gauche_sprites_Amiga_Shadow + largeur_Shadow * pos_y_dest_sprite_Amiga_Shadow;
		for (int y=0;y<hauteur_sprite_Amiga_Shadow;y++)
		{
			for (int x=0;x<largeur_sprite_Amiga_Shadow+pos_x_dest_sprite_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow;x++)
			{
				pixel = pointeur_Amiga_Shadow_u32[pos_source];
				if (pixel != 0xFFB6916D) buffer_ecran_Shadow[pos_dest] = pixel;
				pos_source++;
				pos_dest++;
			}
			pos_source = pos_source + largeur_png_sprites_Amiga_Shadow - ( largeur_sprite_Amiga_Shadow+pos_x_dest_sprite_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow);
			pos_dest = pos_dest + largeur_Shadow - ( largeur_sprite_Amiga_Shadow+pos_x_dest_sprite_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow);
		}			
	}
	// clipping droite
	else if ((pos_x_dest_sprite_Amiga_Shadow+largeur_sprite_Amiga_Shadow) > bord_droit_sprites_Amiga_Shadow )
		{
			int pos_source = pos_y_source_sprite_Amiga_Shadow*largeur_png_sprites_Amiga_Shadow+pos_x_source_sprite_Amiga_Shadow;
			int pos_dest = pos_x_dest_sprite_Amiga_Shadow + largeur_Shadow * pos_y_dest_sprite_Amiga_Shadow;
			int nb_pixels_x_a_afficher = bord_droit_sprites_Amiga_Shadow - pos_x_dest_sprite_Amiga_Shadow ;
			// TRACE("nb_pixels_x_a_afficher : %d\n",nb_pixels_x_a_afficher);
			for (int y=0;y<hauteur_sprite_Amiga_Shadow;y++)
			{
				for (int x=0;x<nb_pixels_x_a_afficher;x++)
				{
					pixel = pointeur_Amiga_Shadow_u32[pos_source];
					if (pixel != 0xFFB6916D) buffer_ecran_Shadow[pos_dest] = pixel;
					pos_source++;
					pos_dest++;
				}
				pos_source = pos_source + largeur_png_sprites_Amiga_Shadow - nb_pixels_x_a_afficher;
				pos_dest = pos_dest + largeur_Shadow  - nb_pixels_x_a_afficher ;
			}
		}
		else
		{
			int pos_dest = pos_x_dest_sprite_Amiga_Shadow + largeur_Shadow * pos_y_dest_sprite_Amiga_Shadow;
			int pos_source = pos_y_source_sprite_Amiga_Shadow*largeur_png_sprites_Amiga_Shadow+pos_x_source_sprite_Amiga_Shadow;
			for (int y=0;y<hauteur_sprite_Amiga_Shadow;y++)
			{
				for (int x=0;x<largeur_sprite_Amiga_Shadow;x++)
				{
					pixel = pointeur_Amiga_Shadow_u32[pos_source];
					if (pixel != 0xFFB6916D) buffer_ecran_Shadow[pos_dest] = pixel;
					pos_source++;
					pos_dest++;
				}
				pos_source = pos_source + largeur_png_sprites_Amiga_Shadow - largeur_sprite_Amiga_Shadow;
				pos_dest = pos_dest + largeur_Shadow - largeur_sprite_Amiga_Shadow;
			}
		}
	
		//pos_x_dest_sprite_Amiga_Shadow = pos_x_dest_sprite_Amiga_Shadow - 2;
		//if ((pos_x_dest_sprite_Amiga_Shadow+largeur_sprite_Amiga_Shadow)< bord_gauche_sprites_Amiga_Shadow) 
		//{
		//	pos_x_dest_sprite_Amiga_Shadow = bord_droit_sprites_Amiga_Shadow;
		//}
}

static void affiche_arbre_gauche_Amiga_Shadow()
{


	
	
	// int pos_x_dest_sprite_arbre_gauche_Amiga_Shadow = 240;
	
	
	// int pos_source = pos_y_source_sprite_arbre_gauche_Amiga_Shadow*largeur_png_sprites_Amiga_Shadow;
	// int pos_dest = pos_x_dest_sprite_arbre_gauche_Amiga_Shadow + largeur_Shadow * pos_y_dest_sprite_arbre_gauche_Amiga_Shadow;
	u32 pixel;
	
	// completement a gauche, invisible
	if ((pos_x_dest_sprite_arbre_gauche_Amiga_Shadow + largeur_sprite_arbre_gauche_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow) < 0) return;
	
	// clipping gauche à 48 pixels
	if ((pos_x_dest_sprite_arbre_gauche_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow) <0 )
	{
		int pos_source = pos_y_source_sprite_arbre_gauche_Amiga_Shadow*largeur_png_sprites_Amiga_Shadow-(pos_x_dest_sprite_arbre_gauche_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow);
		int pos_dest = bord_gauche_sprites_Amiga_Shadow + largeur_Shadow * pos_y_dest_sprite_arbre_gauche_Amiga_Shadow;
		for (int y=0;y<hauteur_sprite_arbre_gauche_Amiga_Shadow;y++)
		{
			for (int x=0;x<largeur_sprite_arbre_gauche_Amiga_Shadow+pos_x_dest_sprite_arbre_gauche_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow;x++)
			{
				pixel = pointeur_Amiga_Shadow_u32[pos_source];
				if (pixel != 0xFFB6916D) buffer_ecran_Shadow[pos_dest] = pixel;
				pos_source++;
				pos_dest++;
			}
			pos_source = pos_source + largeur_png_sprites_Amiga_Shadow - ( largeur_sprite_arbre_gauche_Amiga_Shadow+pos_x_dest_sprite_arbre_gauche_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow);
			pos_dest = pos_dest + largeur_Shadow - ( largeur_sprite_arbre_gauche_Amiga_Shadow+pos_x_dest_sprite_arbre_gauche_Amiga_Shadow-bord_gauche_sprites_Amiga_Shadow);
		}			
	}
	// clipping droite
	else if ((pos_x_dest_sprite_arbre_gauche_Amiga_Shadow+largeur_sprite_arbre_gauche_Amiga_Shadow) > bord_droit_sprites_Amiga_Shadow )
		{
			int pos_source = pos_y_source_sprite_arbre_gauche_Amiga_Shadow*largeur_png_sprites_Amiga_Shadow;
			int pos_dest = pos_x_dest_sprite_arbre_gauche_Amiga_Shadow + largeur_Shadow * pos_y_dest_sprite_arbre_gauche_Amiga_Shadow;
			int nb_pixels_x_a_afficher = bord_droit_sprites_Amiga_Shadow - pos_x_dest_sprite_arbre_gauche_Amiga_Shadow ;
			// TRACE("nb_pixels_x_a_afficher : %d\n",nb_pixels_x_a_afficher);
			for (int y=0;y<hauteur_sprite_arbre_gauche_Amiga_Shadow;y++)
			{
				for (int x=0;x<nb_pixels_x_a_afficher;x++)
				{
					pixel = pointeur_Amiga_Shadow_u32[pos_source];
					if (pixel != 0xFFB6916D) buffer_ecran_Shadow[pos_dest] = pixel;
					pos_source++;
					pos_dest++;
				}
				pos_source = pos_source + largeur_png_sprites_Amiga_Shadow - nb_pixels_x_a_afficher;
				pos_dest = pos_dest + largeur_Shadow  - nb_pixels_x_a_afficher ;
			}
		}
		else
		{
			int pos_dest = pos_x_dest_sprite_arbre_gauche_Amiga_Shadow + largeur_Shadow * pos_y_dest_sprite_arbre_gauche_Amiga_Shadow;
			int pos_source = pos_y_source_sprite_arbre_gauche_Amiga_Shadow*largeur_png_sprites_Amiga_Shadow;
			for (int y=0;y<hauteur_sprite_arbre_gauche_Amiga_Shadow;y++)
			{
				for (int x=0;x<largeur_sprite_arbre_gauche_Amiga_Shadow;x++)
				{
					pixel = pointeur_Amiga_Shadow_u32[pos_source];
					if (pixel != 0xFFB6916D) buffer_ecran_Shadow[pos_dest] = pixel;
					pos_source++;
					pos_dest++;
				}
				pos_source = pos_source + largeur_png_sprites_Amiga_Shadow - largeur_sprite_arbre_gauche_Amiga_Shadow;
				pos_dest = pos_dest + largeur_Shadow - largeur_sprite_arbre_gauche_Amiga_Shadow;
			}
		}
	

}

// routines sprites
static void affiche_sprites_Shadow()
{
	float c;
	int pos_x,pos_y,pos_dest,pos_source;
	for (int i = 0 ; i < 6 ; i++)
	{
		c = (float) (iteration_Shadow + (i * 4.0f));
		pos_x = (72 + i * 48);
		pos_y = (int) (260 + 8 * sin(c / 20.0f) * sin(c / 8.0f));
		
		pos_source = i*largeur_sprites_Shadow;
		pos_dest = pos_x + ( pos_y * largeur_Shadow);
		
		for (int y=0;y<hauteur_sprites_Shadow;y++)
		{
			for (int x=0;x<largeur_sprites_Shadow;x++)
			{
				buffer_ecran_Shadow[pos_dest] = pointeur_sprites_Shadow_u32[pos_source];
				pos_source++;
				pos_dest++;
			}
			pos_source = pos_source + (largeur_sprites_Shadow * 6) - largeur_sprites_Shadow;
			pos_dest = pos_dest + largeur_Shadow - largeur_sprites_Shadow;
		}
		
	}
		
		
	
	iteration_Shadow++;
}

// routine parallax
static void affiche_parallax_Shadow(int numero_parallax)
{
		int pos_source2,pos_dest2;
		int pos_source = parallax[numero_parallax].posy*largeur_background_Shadow;
		int pos_dest = parallax[numero_parallax].posy*largeur_Shadow+48+parallax[numero_parallax].posx_actuelle;
		
		// a droite
		for (int y=0;y<parallax[numero_parallax].hauteur;y++)
		{
			pos_source2=pos_source;
			pos_dest2 = pos_dest;
			for (int x=0;x<largeur_background_Shadow-parallax[numero_parallax].posx_actuelle;x++)
			{
				buffer_ecran_Shadow[pos_dest2] = pointeur_background_Shadow_u32[pos_source2];
				pos_source2++;
				pos_dest2++;
			}
			pos_source = pos_source + largeur_background_Shadow ;
			pos_dest = pos_dest + largeur_Shadow;
		}
		
		// a gauche
		pos_source = parallax[numero_parallax].posy*largeur_background_Shadow+( largeur_background_Shadow - parallax[numero_parallax].posx_actuelle);
		pos_dest = parallax[numero_parallax].posy*largeur_Shadow+48;
		for (int y=0;y<parallax[numero_parallax].hauteur;y++)
		{
			pos_source2=pos_source;
			pos_dest2 = pos_dest;
			for (int x=0;x<parallax[numero_parallax].posx_actuelle;x++)
			{
				buffer_ecran_Shadow[pos_dest2] = pointeur_background_Shadow_u32[pos_source2];
				pos_source2++;
				pos_dest2++;
			}
			pos_source = pos_source + largeur_background_Shadow ;
			pos_dest = pos_dest + largeur_Shadow;
		}
		
		
		parallax[numero_parallax].posx_actuelle = parallax[numero_parallax].posx_actuelle - parallax[numero_parallax].increment;
		if ( parallax[numero_parallax].posx_actuelle <= 0) parallax[numero_parallax].posx_actuelle =   largeur_background_Shadow - parallax[numero_parallax].posx_actuelle;
}

// routine overlay
static void affiche_overlay_Shadow(int positionverticale)
{
	int pos_source=positionverticale*largeur_overlay_Shadow;
	int pos_dest=10 * largeur_Shadow;
	u32 pixel;
		for (int y=0;y<hauteur_Shadow-10;y++)
		{
			for (int x=0;x<largeur_Shadow;x++)
			{
				pixel = pointeur_overlay_Shadow_u32[pos_source];
				if (pixel != 0x00000000) buffer_ecran_Shadow[pos_dest] = pixel;
				pos_source++;
				pos_dest++;
			}
		}
				
}




	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[])

{
	/////////////////////////////// INTRO //////////////////////////////////////////////////////
		Thread id_thread_playmodule_Intro;
	threadCreate(&id_thread_playmodule_Intro, remplissage_module_intro, NULL, NULL, 0x8000, 0x30, 2);
    
	
	for (int i=0;i<nombre_de_flocons;i++)	
	{
		flocons[i].delai_avant_dispo = (rand() % 180);
		flocons[i].actif =0;
	}
		
	
	
	pointeur_logo_nintendo = stbi_load_from_memory((const stbi_uc*) Nintendo2_png, Nintendo2_png_size, &width, &height, &nchan, 4);
	pointeur_logo_Nintendo_u32 = (u32*) pointeur_logo_nintendo;
	
		
	pointeur_logo_Atari = stbi_load_from_memory((const stbi_uc*) atari_png, atari_png_size, &width, &height, &nchan, 4);
	pointeur_logo_Atari_u32 = (u32*) pointeur_logo_Atari;
	
	
	
	// 2 buffers pour bosser
	buffer_ecran_source = (u32*) malloc(FB_WIDTH * FB_HEIGHT * 4);
	memset(buffer_ecran_source,0,FB_WIDTH * FB_HEIGHT * 4);
	buffer_ecran_dest = (u32*) malloc(FB_WIDTH * FB_HEIGHT * 4);
	memset(buffer_ecran_dest,0,FB_WIDTH * FB_HEIGHT * 4);
	
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Configure our supported input layout: a single player with standard controller styles
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);

	// Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
	//PadState pad;
	//padInitializeDefault(&pad);
	
	
	// Retrieve the default window
	NWindow* win = nwindowGetDefault();

	
	// Create a linear double-buffered framebuffer
	Framebuffer fb;
	framebufferCreate(&fb, win, FB_WIDTH, FB_HEIGHT, PIXEL_FORMAT_RGBA_8888, 2);
	framebufferMakeLinear(&fb);
	
			// Retrieve the framebuffer
		framebuf = (u32*) framebufferBegin(&fb, &stride);
		previousTick = armGetSystemTick();

		int pos_source=0;
		int pos_dest=position_x_logo_nintendo+(position_y_logo_nintendo*FB_WIDTH);
		for (int y=0;y<hauteur_logo_Nintendo;y++)
		{
			for (int x=0;x<largeur_logo_Nintendo;x++)
			{
				u32 pixel = pointeur_logo_Nintendo_u32[pos_source];
				if (pixel != 0x00000000 and pixel !=0xFF000000) nombre_pixels_logo_Nintendo++;
				framebuf[pos_dest]=pixel;
				pos_source++;
				pos_dest++;
			}
			pos_dest=pos_dest+FB_WIDTH-largeur_logo_Nintendo;
		}
		
		// Vsync
		framebufferEnd(&fb);

	
	svcSleepThread(17000000*60*6);
		
	// pause



	// monter le logo en hauteur
	for (int pos_y_en_cours = position_y_logo_nintendo;pos_y_en_cours>=0;pos_y_en_cours--)
	{
				// Retrieve the framebuffer
		framebuf = (u32*) framebufferBegin(&fb, &stride);
		previousTick = armGetSystemTick();
		
		int pos_source=0;
		int pos_dest=position_x_logo_nintendo+(pos_y_en_cours*FB_WIDTH);
		for (int y=0;y<hauteur_logo_Nintendo;y++)
		{
			for (int x=0;x<largeur_logo_Nintendo;x++)
			{
				framebuf[pos_dest]=pointeur_logo_Nintendo_u32[pos_source];
				pos_source++;
				pos_dest++;
			}
			pos_dest=pos_dest+FB_WIDTH-largeur_logo_Nintendo;
		}

		// Vsync
		framebufferEnd(&fb);
	}

	// remplir les buffers
	pos_source=0;
	pos_dest=position_x_logo_nintendo;
	for (int y=0;y<hauteur_logo_Nintendo;y++)
		{
			for (int x=0;x<largeur_logo_Nintendo;x++)
			{
				buffer_ecran_dest[pos_dest]=pointeur_logo_Nintendo_u32[pos_source];
				buffer_ecran_source[pos_dest]=pointeur_logo_Nintendo_u32[pos_source];
				
				pos_source++;
				pos_dest++;
			}
			pos_dest=pos_dest+FB_WIDTH-largeur_logo_Nintendo;
		}
	
	
	
	threadStart(&id_thread_playmodule_Intro);
	
	
		// Main loop decomposition logo Nintendo
	u32 pointeur_dernier_point;
	while (appletMainLoop())
	{
		// Scan the gamepad. This should be done once for each frame
		//padUpdate(&pad);

		// padGetButtonsDown returns the set of buttons that have been
		// newly pressed in this frame compared to the previous one
		//u64 kDown = padGetButtonsDown(&pad);

		//if (kDown & HidNpadButton_Plus)
		//	break; // break in order to return to hbmenu

		// Retrieve the framebuffer
		framebuf = (u32*) framebufferBegin(&fb, &stride);
		previousTick = armGetSystemTick();


		
		
		// tirage aléatoire de N flocon
		for (int i=0;i<nombre_de_flocons;i++)
		{
			if (flocons[i].actif == 0)
			{
				if (flocons[i].delai_avant_dispo <= 0)
				{
					int posx_aleatoire = (int) (randomGet64() % largeur_fenetre_recherche_de_point )+x_min_fenetre_recherche_de_point;
					//int posx_aleatoire = (int) (randomGet64() % FB_WIDTH)+position_x_logo_nintendo;
					int posy_aleatoire = (int) (randomGet64() % hauteur_fenetre_recherche_de_point )+y_min_fenetre_recherche_de_point;
					//TRACE("posx_aleatoire : %d\n",posx_aleatoire);
					//TRACE("pixel : %X\n", buffer_ecran_source[posx_aleatoire+(posy_aleatoire*FB_WIDTH)]);
					if (buffer_ecran_source[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] != 0x00000000 and buffer_ecran_source[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] != 0xFF000000)
					{
						// le pixel n'est pas noir
						
						// le pixel en dessous est il noir ? et on est pas en bas 
						if ((buffer_ecran_source[posx_aleatoire+((posy_aleatoire+1)*FB_WIDTH)] == 0x00000000 or buffer_ecran_source[posx_aleatoire+((posy_aleatoire+1)*FB_WIDTH)] == 0xFF000000) and (posy_aleatoire+1 < FB_HEIGHT))
						{
							
							//TRACE("pixel avant : %X\n",buffer_ecran_dest[posx_aleatoire+(posy_aleatoire*FB_WIDTH)]);
							//TRACE("posx_aleatoire : %d\n",posx_aleatoire);
							//TRACE("posy_aleatoire : %d\n",posy_aleatoire);
							buffer_ecran_dest[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] = 0x00000000;
							buffer_ecran_source[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] = 0x00000000;
							buffer_ecran_dest[posx_aleatoire+((posy_aleatoire+1)*FB_WIDTH)] = 0xFFFFFFFF;
							//TRACE("pixel final : %X\n",buffer_ecran_dest[posx_aleatoire+(posy_aleatoire*FB_WIDTH)]);
						}
						else if ((buffer_ecran_source[posx_aleatoire-1+((posy_aleatoire+1)*FB_WIDTH)] == 0x00000000 or buffer_ecran_source[posx_aleatoire - 1 +((posy_aleatoire+1)*FB_WIDTH)] == 0xFF000000) and (posy_aleatoire+1 < FB_HEIGHT) and (posx_aleatoire-1 >=0))
						{
								
							buffer_ecran_dest[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] = 0x00000000;
							buffer_ecran_source[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] = 0x00000000;
							buffer_ecran_dest[posx_aleatoire-1+((posy_aleatoire+1)*FB_WIDTH)] = 0xFFFFFFFF;
							posx_aleatoire = posx_aleatoire -1;
						}
							else if ((buffer_ecran_source[posx_aleatoire+1+((posy_aleatoire+1)*FB_WIDTH)] == 0x00000000 or buffer_ecran_source[posx_aleatoire + 1 +((posy_aleatoire+1)*FB_WIDTH)] == 0xFF000000) and (posy_aleatoire+1 < FB_HEIGHT) and (posx_aleatoire+1 < FB_WIDTH))
						{
								
							buffer_ecran_dest[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] = 0x00000000;
							buffer_ecran_source[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] = 0x00000000;
							buffer_ecran_dest[posx_aleatoire+1+((posy_aleatoire+1)*FB_WIDTH)] = 0xFFFFFFFF;
							posx_aleatoire = posx_aleatoire +1;
						}
							
						flocons[i].posx = posx_aleatoire;
						flocons[i].posy = posy_aleatoire+1;
						flocons[i].actif = 1;
						nombre_points_actifs++;
						flocons[i].vitesse = (randomGet64()  %3)+1;
					}
						
				}
				else flocons[i].delai_avant_dispo--;
			}
			else
				// flocon est actif
				{
					int posx_en_cours = flocons[i].posx;
					int posy_en_cours = flocons[i].posy;
					if ((buffer_ecran_source[posx_en_cours+((posy_en_cours+1)*FB_WIDTH)] == 0x00000000 or buffer_ecran_source[posx_en_cours+((posy_en_cours+1)*FB_WIDTH)] == 0xFF000000) and (posy_en_cours+flocons[i].vitesse < FB_HEIGHT))
						{
							//TRACE("avance un flocon : %d\n",i);
							flocons[i].posy = posy_en_cours +flocons[i].vitesse;
							buffer_ecran_dest[posx_en_cours+(posy_en_cours*FB_WIDTH)] = 0x00000000;
							buffer_ecran_source[posx_en_cours+(posy_en_cours*FB_WIDTH)] = 0x00000000;
							pointeur_dernier_point = posx_en_cours+((posy_en_cours+flocons[i].vitesse)*FB_WIDTH);
							buffer_ecran_dest[posx_en_cours+((posy_en_cours+flocons[i].vitesse)*FB_WIDTH)] = 0xFFFFFFFF;
						}
						// a gauche
					else if ((buffer_ecran_source[posx_en_cours-1+((posy_en_cours+1)*FB_WIDTH)] == 0x00000000 or buffer_ecran_source[posx_en_cours - 1 +((posy_en_cours+1)*FB_WIDTH)] == 0xFF000000) and (posy_en_cours+flocons[i].vitesse < FB_HEIGHT) and (posx_en_cours - 1 >=0))
						{
							flocons[i].posx = posx_en_cours -1;
							flocons[i].posy = posy_en_cours + flocons[i].vitesse;
							buffer_ecran_source[posx_en_cours+(posy_en_cours*FB_WIDTH)] = 0x00000000;
							buffer_ecran_dest[posx_en_cours+(posy_en_cours*FB_WIDTH)] = 0x00000000;
							pointeur_dernier_point=posx_en_cours-1+((posy_en_cours+flocons[i].vitesse)*FB_WIDTH);
							buffer_ecran_dest[posx_en_cours-1+((posy_en_cours+flocons[i].vitesse)*FB_WIDTH)] = 0xFFFFFFFF;
						}	
						else if ((buffer_ecran_source[posx_en_cours+1+((posy_en_cours+1)*FB_WIDTH)] == 0x00000000 or buffer_ecran_source[posx_en_cours + 1 +((posy_en_cours+1)*FB_WIDTH)] == 0xFF000000) and (posy_en_cours+flocons[i].vitesse < FB_HEIGHT) and (posx_en_cours+1<FB_WIDTH))
						{
							flocons[i].posx = posx_en_cours +1;
							flocons[i].posy = posy_en_cours +flocons[i].vitesse;
							buffer_ecran_source[posx_en_cours+(posy_en_cours*FB_WIDTH)] = 0x00000000;
							buffer_ecran_dest[posx_en_cours+(posy_en_cours*FB_WIDTH)] = 0x00000000;
							pointeur_dernier_point=posx_en_cours+1+((posy_en_cours+flocons[i].vitesse)*FB_WIDTH);
							buffer_ecran_dest[posx_en_cours+1+((posy_en_cours+flocons[i].vitesse)*FB_WIDTH)] = 0xFFFFFFFF;
						}	
						// sinon bloqué donc plus actif
						else
						{
							if ( flocons[i].vitesse > 1) flocons[i].vitesse=1;
							else
							{
								int posx_aleatoire = (int) (randomGet64() % largeur_fenetre_recherche_de_point )+x_min_fenetre_recherche_de_point;
								int posy_aleatoire = (int) (randomGet64() % hauteur_fenetre_recherche_de_point )+y_min_fenetre_recherche_de_point;
								if (buffer_ecran_source[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] != 0x00000000 and buffer_ecran_source[posx_aleatoire+(posy_aleatoire*FB_WIDTH)] != 0xFF000000)
								{
									flocons[i].posx = posx_aleatoire;
									flocons[i].posy = posy_aleatoire;
									flocons[i].actif = 1;
									flocons[i].vitesse = (randomGet64()  %3)+1;
								}
								else
								{
									flocons[i].actif = 0;
									nombre_points_actifs--;
								}
							}
						}
				}		
		}
		

			

		// recopie buffe dest dans framebuf
		int pos_source=0;
		int pos_dest=0;
		for (int y=0;y<FB_HEIGHT;y++)
		{
			for (int x=0;x<FB_WIDTH;x++)
			{
				framebuf[pos_dest]=buffer_ecran_dest[pos_source];
				pos_source++;
				pos_dest++;
			}
			
		}
		// recopie buffe dest dans buffer source
		pos_source=0;
		pos_dest=0;
		for (int y=0;y<FB_HEIGHT;y++)
		{
			for (int x=0;x<FB_WIDTH;x++)
			{
				buffer_ecran_source[pos_dest]=buffer_ecran_dest[pos_source];
				pos_source++;
				pos_dest++;
			}
			
		}
		// Vsync
		framebufferEnd(&fb);
	
		
		if (nombre_points_actifs > 2000 and ca_a_commence ==0) ca_a_commence=1;
		if (nombre_points_actifs < 500 and ca_a_commence ==1) c_est_termine=1;
		
		if (c_est_termine == 1)
		{
			min_y=FB_HEIGHT;
			for (int i=0;i<nombre_de_flocons;i++)
			{
				if (flocons[i].posy < min_y) min_y=flocons[i].posy;
			}
			
			
			//TRACE("min_y : %d\n",min_y);
			if  (min_y > min_y_precedent  ) 
			{
				min_y_precedent=min_y;
			}
			else
			{
				if ( min_y >500) 
				{
					break;
				}
			}
		}
		
		//TRACE("boucle vbl : %d\n",nombre_points_actifs);


	
	}

		// efface le dernier point
	buffer_ecran_dest[pointeur_dernier_point]=0;
	buffer_ecran_source[pointeur_dernier_point]=0;
	framebuf[pointeur_dernier_point]=0;

	diviseur_volume_global=2;


	
	

	
	// logo Atari
	// x et y aléatoires , source, si pixel <> 0 
	// x et y aléatoires destination, si pixel <> 0
	// vitesse aléatoire
	// calcul incrément x et y / vitesse
	// framebuffer dans buffer_source
	// logo atari dans buffer_dest
	
	
	
	
	// framebuffer dans buffer_ecran_source
	memcpy(buffer_ecran_source, buffer_ecran_dest, FB_WIDTH * FB_HEIGHT * 4);

	

	// compter le nombre de pixels du logo Nintendo détruit
	int nb_points_logo_detruit=0;
	pos_source=0;
	for (int y=0;y<FB_HEIGHT;y++)
	{
		for (int x=0;x<FB_WIDTH;x++)
		{
			u32 pixel = buffer_ecran_source[pos_source];
			pos_source++;
			if ( (pixel != 0x00000000) and (pixel != 0xFF000000) and (pixel != 0xFF070A0D)) 
			{
				
				points_logo_detruit[nb_points_logo_detruit].posx = x;
				points_logo_detruit[nb_points_logo_detruit].posy = y;
				nb_points_logo_detruit++;
			}
			
		}
	}
	
	
	
	// buffer dest = 0
	memset(buffer_ecran_dest, 0, FB_WIDTH * FB_HEIGHT * 4);

	// nouveau buffer final
	//u32* buffer_final = (u32*) malloc(FB_WIDTH * FB_HEIGHT * 4);
	//memset(buffer_final, 0, FB_WIDTH * FB_HEIGHT * 4);

	
	// copie logo Atari dans dest
	pos_source=0;
	pos_dest=position_x_logo_Atari + ( position_y_logo_Atari*FB_WIDTH);
	
	for (int y=0;y<hauteur_logo_Atari;y++)
	{
		for (int x=0;x<largeur_logo_Atari;x++)
		{
			buffer_ecran_dest[pos_dest] = pointeur_logo_Atari_u32[pos_source];
			pos_source++;
			pos_dest++;
		}
		pos_dest=pos_dest+FB_WIDTH-largeur_logo_Atari;
	}
	

	// compter le nombre de pixel du logo Atari
	int nb_points_logo_Atari=0;
	pos_source=0;
	for (int y=0;y<FB_HEIGHT;y++)
	{
		for (int x=0;x<FB_WIDTH;x++)
		{
			u32 pixel = buffer_ecran_dest[pos_source];
			pos_source++;
			if ( (pixel != 0x00000000) and (pixel != 0xFF000000) and (pixel != 0xFF070A0D)) 
			{
				//flocons[nb_points_logo_Atari].vitesse = (randomGet64()  %3)+1;
				nb_points_logo_Atari++;
			}
			
		}
	}
	
	
	
	// tableau des points du logo Atari
	// struct : flocon
	// on prend juste X et Y en linéaire
	// vérifié OK
	
	pos_source=0;
	u64 pos_dest2=0;
	nb_points_logo_Atari=0;
	for (int y=position_y_logo_Atari; y< (hauteur_logo_Atari +hauteur_logo_Atari ); y++)
	{
		for (int x=position_x_logo_Atari; x<(position_x_logo_Atari +largeur_logo_Atari ) ; x++)
		{
			u32 pixel = buffer_ecran_dest[x+(y*FB_WIDTH)];
			if ( (pixel != 0x00000000) and (pixel != 0xFF000000) and (pixel != 0xFF070A0D)) 
			{
				nb_points_logo_Atari++;
				points_logo_Atari[pos_dest2].posx_destination_finale=x;
				points_logo_Atari[pos_dest2].posy_destination_finale=y;
				points_logo_Atari[pos_dest2].n_a_pas_demarre = 1;
				points_logo_Atari[pos_dest2].actif=1;
				points_logo_Atari[pos_dest2].couleur_finale = pixel;
				points_logo_Atari[pos_dest2].delai_avant_dispo = (rand() % 180);
				pos_dest2++;
			}
		}
	}
	// on a un tableau linéaire des points du logo
	//TRACE("2eme comptage nb_points_logo_Atari : %d\n",nb_points_logo_Atari);
	//TRACE("2eme comptage pos_dest2 : %d\n",pos_dest2);
	
	// tableau linéaire des points du bas
	// nb_points_logo_detruit
	// dans points_logo_detruit avec posx et posy
	
	
	
	
	// stratégie :
	//- affecter de facon linéaire un point du bas a un point du logo, 
	//- tout mélanger longtemps : tirer au hasard un numéro de point haut, un 2eme numéro de point haut , échanger leurs sources de point bas
	// - a la fin , parcourir les points du logo Atari, et calculer pour chaque point si le point bas est réutilisé par la suite
	
	
	//- affecter de facon linéaire un point du bas a un point du logo, 
	int index_dans_logo_detruit =1;
	// de nb_points_logo_Atari à nb_points_logo_detruit
	// on affecte en linéaire
	
	for (int i = nb_points_logo_detruit;i< nb_points_logo_Atari; i++)
	{
		index_dans_logo_detruit = (randomGet64() % nb_points_logo_detruit);
		points_logo_Atari[i].posx = points_logo_detruit[index_dans_logo_detruit].posx;
		points_logo_Atari[i].posy = points_logo_detruit[index_dans_logo_detruit].posy;
		points_logo_Atari[i].posx_actuelle_avec_virgule = (float) points_logo_detruit[index_dans_logo_detruit].posx;
		points_logo_Atari[i].posy_actuelle_avec_virgule = (float) points_logo_detruit[index_dans_logo_detruit].posy;
	}
		
	// les derniers points , nb points en bas
	for (int i = 0;i< nb_points_logo_detruit; i++)
	{ 
		// index_dans_logo_detruit = nb_points_logo_detruit - (i % nb_points_logo_detruit);
		index_dans_logo_detruit = i;
		// index_dans_logo_detruit =1;
		///
		
		points_logo_Atari[i].posx = points_logo_detruit[index_dans_logo_detruit].posx;
		points_logo_Atari[i].posy = points_logo_detruit[index_dans_logo_detruit].posy;
		points_logo_Atari[i].posx_actuelle_avec_virgule = (float) points_logo_detruit[index_dans_logo_detruit].posx;
		points_logo_Atari[i].posy_actuelle_avec_virgule = (float) points_logo_detruit[index_dans_logo_detruit].posy;
		
	}
	
	
	
	// on melange les nb_points_logo_detruit derniers points
	for (int iteration_melange = 0;iteration_melange < 10;iteration_melange++)
	{
		for (int i = 0;i< nb_points_logo_detruit; i++)
		{
			int index_point1 = (randomGet64() % nb_points_logo_detruit);
			int index_point2 = (randomGet64() % nb_points_logo_detruit);
			
			// on inverse les points source
			struct flocon temp[1];
			temp[0].posx = (float) points_logo_Atari[index_point1].posx;
			temp[0].posy = (float) points_logo_Atari[index_point1].posy;
			temp[0].posx_actuelle_avec_virgule = (float) points_logo_Atari[index_point1].posx_actuelle_avec_virgule;
			temp[0].posy_actuelle_avec_virgule = (float) points_logo_Atari[index_point1].posy_actuelle_avec_virgule;

			points_logo_Atari[index_point1].posx = points_logo_Atari[index_point2].posx;
			points_logo_Atari[index_point1].posy = points_logo_Atari[index_point2].posy;
			points_logo_Atari[index_point1].posx_actuelle_avec_virgule = points_logo_Atari[index_point2].posx_actuelle_avec_virgule;
			points_logo_Atari[index_point1].posy_actuelle_avec_virgule = points_logo_Atari[index_point2].posy_actuelle_avec_virgule;			
			
			points_logo_Atari[index_point2].posx = temp[0].posx;
			points_logo_Atari[index_point2].posy = temp[0].posy;
			points_logo_Atari[index_point2].posx_actuelle_avec_virgule = temp[0].posx_actuelle_avec_virgule;
			points_logo_Atari[index_point2].posy_actuelle_avec_virgule = temp[0].posy_actuelle_avec_virgule;
			
		}
	}
		
		
		
	
	
	
	
	//- tout mélanger longtemps : tirer au hasard un numéro de point haut, un 2eme numéro de point haut , échanger leurs sources de point bas
	
	for (int iteration_melange = 0;iteration_melange < 10;iteration_melange++)
	{
		for (int i = 0;i< nb_points_logo_Atari; i++)
		{
			int index_point1 = (randomGet64() % nb_points_logo_Atari);
			int index_point2 = (randomGet64() % nb_points_logo_Atari);
		
			// on inverse les points de destination
			struct flocon temp[1];
			temp[0].posx_destination_finale = (float) points_logo_Atari[index_point1].posx_destination_finale;
			temp[0].posy_destination_finale = (float) points_logo_Atari[index_point1].posy_destination_finale;
			temp[0].couleur_finale=points_logo_Atari[index_point1].couleur_finale;
		
			points_logo_Atari[index_point1].posx_destination_finale = points_logo_Atari[index_point2].posx_destination_finale;
			points_logo_Atari[index_point1].posy_destination_finale = points_logo_Atari[index_point2].posy_destination_finale;
			points_logo_Atari[index_point1].couleur_finale = points_logo_Atari[index_point2].couleur_finale;
		
			points_logo_Atari[index_point2].posx_destination_finale = temp[0].posx_destination_finale;
			points_logo_Atari[index_point2].posy_destination_finale = temp[0].posy_destination_finale;
			points_logo_Atari[index_point2].couleur_finale = temp[0].couleur_finale;
			
		}
	}
	
	for (int i=2;i<25;i=i+1)
	{
		diviseur_volume_global=i;
		svcSleepThread(70000000*2); 
	}

	diviseur_volume_global=60;
	

	// - a la fin , parcourir les points du logo Atari, et calculer pour chaque point si le point bas est réutilisé par la suite
	for (int i = 0;i< nb_points_logo_Atari; i++)
	{
		int est_reutilise = 0;
		int point_bas_x = points_logo_Atari[i].posx;
		int point_bas_y = points_logo_Atari[i].posy;
		
		for (int j=i+1;j<nb_points_logo_Atari;j++)
		{
			if ( points_logo_Atari[j].posx == point_bas_x and points_logo_Atari[j].posy == point_bas_y)
			{
				est_reutilise++;
			}
		}
		// apres la boucle de recherche
		points_logo_Atari[i].point_source_est_reutilise = est_reutilise;
		//TRACE("points_logo_Atari[i].point_source_est_reutilise : %d\n",points_logo_Atari[i].point_source_est_reutilise);
	}
	
	
	
	// recalculer nb etapes et increment
	int nb_etapes=1;
	int deltax,deltay;
	int vitesse;
	for (int i = 0;i< nb_points_logo_Atari; i++)
	{
		points_logo_Atari[i].vitesse = (int) ((randomGet64()  % 3 )+1);
		vitesse = points_logo_Atari[i].vitesse;
		
		deltax =  points_logo_Atari[i].posx_destination_finale - points_logo_Atari[i].posx;
		deltay =  points_logo_Atari[i].posy_destination_finale - points_logo_Atari[i].posy;
		
		
		nb_etapes = (int) abs(180 / vitesse);
		points_logo_Atari[i].nb_etapes = nb_etapes;
									
		points_logo_Atari[i].incrementx = (float) deltax / nb_etapes;
		points_logo_Atari[i].incrementy = (float) deltay / nb_etapes;
		
	}

	

	// int nombre_pixels_en_surplus = nb_points_logo_Atari - nb_points_logo_detruit;
	//TRACE("nombre_pixels_en_surplus : %d\n",nombre_pixels_en_surplus);
	
	// sauvegarde ecran pour CLS
	u32* buffer_ecran_CLS = (u32*) malloc(FB_WIDTH*FB_HEIGHT*4);
	
	// on copie dans sauvegarde pour CLS
	for (int i=0; i<(FB_WIDTH*FB_HEIGHT);i++)	buffer_ecran_CLS[i] = buffer_ecran_source[i];

	numero_musique=2;
	diviseur_volume_global=1;	
	
	#define	nbpointsparvbl	2000

	// deuxieme boucle
	int sequence_logo_Atari_terminee=0;
	int nombre_total_de_points;
	nombre_total_de_points = nb_points_logo_Atari;
	
	while (appletMainLoop())
	{
	
			// Scan the gamepad. This should be done once for each frame
		//padUpdate(&pad);

		// padGetButtonsDown returns the set of buttons that have been
		// newly pressed in this frame compared to the previous one
		//u64 kDown = padGetButtonsDown(&pad);

		//if (kDown & HidNpadButton_Plus)
		//	break; // break in order to return to hbmenu

		// Retrieve the framebuffer
		framebuf = (u32*) framebufferBegin(&fb, &stride);
		
		
		
		// on restaure l ecran
		for (int i=0; i<(FB_WIDTH*FB_HEIGHT);i++)	buffer_ecran_source[i]=buffer_ecran_CLS[i];
		
		// sauvegarde, calcul et affiche
		int nb_points_a_afficher = nbpointsparvbl;
		u32 index_point_en_cours=0;
		while (nb_points_a_afficher>0 and sequence_logo_Atari_terminee==0)
		{
			// check qu'on dépasse pas le nb de points disponibles
			if (index_point_en_cours < nb_points_logo_Atari)
			{
				// le point a depassé son delai d attente ?
				if (points_logo_Atari[index_point_en_cours].delai_avant_dispo ==0)
				{	
					// le point n est pas arrivé a destination ?
					if (points_logo_Atari[index_point_en_cours].nb_etapes > 0)
					{
						// le point est nouveau ?
						if (points_logo_Atari[index_point_en_cours].n_a_pas_demarre == 1)
						{
						
							// c'est un nouveau point qu'il faut avancer
							// le point doit etre definitivement effacé car pas reutilisé
							if (points_logo_Atari[index_point_en_cours].point_source_est_reutilise == 0)
							{
								// le point n est pas reutilisé donc l'efface
								int pos_tmp_x = points_logo_Atari[index_point_en_cours].posx;
								int pos_tmp_y = points_logo_Atari[index_point_en_cours].posy;
								int pos_point_a_effacer = pos_tmp_x + ( pos_tmp_y*FB_WIDTH);
								buffer_ecran_CLS[pos_point_a_effacer]=0x00000000;
							}
							points_logo_Atari[index_point_en_cours].n_a_pas_demarre=0;
						}
						// gerer le point en mouvement
					
						// on calcul l'avancée
						float posactuelx = (float) (points_logo_Atari[index_point_en_cours].posx_actuelle_avec_virgule);
						float posactuely = (float) (points_logo_Atari[index_point_en_cours].posy_actuelle_avec_virgule);
						posactuelx = (float) (posactuelx + points_logo_Atari[index_point_en_cours].incrementx);
						posactuely = (float) (posactuely + points_logo_Atari[index_point_en_cours].incrementy);
						points_logo_Atari[index_point_en_cours].posx_actuelle_avec_virgule = posactuelx;
						points_logo_Atari[index_point_en_cours].posy_actuelle_avec_virgule = posactuely;
					
						// on affiche le point
						int	posactuelx_int = (int) posactuelx;
						int	posactuely_int = (int) posactuely;
						int pos_point_a_afficher = posactuelx_int + ( posactuely_int*FB_WIDTH);
						buffer_ecran_source[pos_point_a_afficher]=0xFFFFFFFF;
					
						// une etape en moins
						points_logo_Atari[index_point_en_cours].nb_etapes--;
						if (points_logo_Atari[index_point_en_cours].nb_etapes == 0)
						{
							// on a affiché toutes les étapes
							int	posactuelx_int = points_logo_Atari[index_point_en_cours].posx_destination_finale;
							int	posactuely_int = points_logo_Atari[index_point_en_cours].posy_destination_finale;
							int pos_point_a_afficher = posactuelx_int + ( posactuely_int*FB_WIDTH);
							buffer_ecran_source[pos_point_a_afficher]=points_logo_Atari[index_point_en_cours].couleur_finale;
							buffer_ecran_CLS[pos_point_a_afficher]=points_logo_Atari[index_point_en_cours].couleur_finale;
							points_logo_Atari[index_point_en_cours].actif=0;
							nombre_total_de_points--;
						}
						nb_points_a_afficher--;
					}
				}
				else
				{
					// on attend le délai
					points_logo_Atari[index_point_en_cours].delai_avant_dispo--;
				}
				index_point_en_cours++;
			}
			else nb_points_a_afficher--;
		}
			
			
			
			
	
		
		// recopie buffe source dans framebuf
		int pos_source=0;
		int pos_dest=0;
		for (int y=0;y<FB_HEIGHT;y++)
		{
			for (int x=0;x<FB_WIDTH;x++)
			{
				framebuf[pos_dest]=buffer_ecran_source[pos_source];
				pos_source++;
				pos_dest++;
			}
			
		}
		
	
		// Vsync
		framebufferEnd(&fb);
	
		if (nombre_total_de_points<=0) 
		{
			sequence_logo_Atari_terminee=1;
			break;
			//TRACE("Termine");
		}
	
	}
	
	// on deplace le logo Atari au milieu
	for (int pos_y_en_cours = position_y_logo_Atari ;pos_y_en_cours<231;pos_y_en_cours++)
	{
				// Retrieve the framebuffer
		framebuf = (u32*) framebufferBegin(&fb, &stride);
		previousTick = armGetSystemTick();
		
		int pos_source=0;
		int pos_dest=position_x_logo_Atari +(pos_y_en_cours*FB_WIDTH);
		for (int y=0;y<hauteur_logo_Atari ;y++)
		{
			for (int x=0;x<largeur_logo_Atari ;x++)
			{
				framebuf[pos_dest]=pointeur_logo_Atari_u32[pos_source];
				pos_source++;
				pos_dest++;
			}
			pos_dest=pos_dest+FB_WIDTH-largeur_logo_Atari ;
		}

		// Vsync
		framebufferEnd(&fb);
	}



	// on zoom le logo
	int xcentre = (int)FB_WIDTH /2;
	int ycentre = (int) FB_HEIGHT / 2;
	
	int xcentre_logo_Atari = (int) largeur_logo_Atari/2;
	int ycentre_logo_Atari = (int) hauteur_logo_Atari/2;
	
	float inc_zoom=0.001;
	for (float zoom_actuel=1;zoom_actuel< 1000;zoom_actuel=zoom_actuel+inc_zoom)
	{
		// Retrieve the framebuffer
		framebuf = (u32*) framebufferBegin(&fb, &stride);
		previousTick = armGetSystemTick();
		
		int x_min = xcentre - ((int) ((float) (largeur_logo_Atari/2) * zoom_actuel));
		int x_max = xcentre + ((int) ((float) (largeur_logo_Atari/2) * zoom_actuel));
		
		float hauteur_zoomee = ((float) hauteur_logo_Atari * zoom_actuel);
		int y_min = ycentre - (int) abs(hauteur_zoomee/2);
		int y_max = ycentre + (int) abs(hauteur_zoomee/2);
		
		if (x_min<0) x_min=0;
		if (y_min<0) y_min=0;
		
		if (x_max>FB_WIDTH) x_max=FB_WIDTH;
		if (y_max>FB_HEIGHT) y_max=FB_HEIGHT;
		
		
		for (int y=y_min;y<y_max;y++)
		{
			for (int x=x_min; x<x_max;x++)
			{
				int xabsolu_dest = (x-xcentre);
				int yabsolu_dest = (y-ycentre);
				int xreduit = (int) (xabsolu_dest / zoom_actuel);
				int yreduit = (int) (yabsolu_dest / zoom_actuel);
				xreduit = xreduit + xcentre_logo_Atari;
				yreduit = yreduit + ycentre_logo_Atari;
				
				framebuf[x+(y*FB_WIDTH)] = pointeur_logo_Atari_u32[ xreduit+(yreduit*largeur_logo_Atari)];
			}
		}
		
		inc_zoom= inc_zoom+ (inc_zoom/4);
		
		// Vsync
		framebufferEnd(&fb);
	}
	


	
	runningIntro=false;
    threadWaitForExit(&id_thread_playmodule_Intro);
    threadClose(&id_thread_playmodule_Intro);
	
	 // Stop audio playback.
    rc = audoutStopAudioOut();
    //printf("audoutStopAudioOut() returned 0x%x\n", rc);

    // Terminate the default audio output device.
    audoutExit();
	
    // Free resources
	ymMusicDestroy(pMusic_Intro);
	ymMusicDestroy(pMusic_Intro2);
	
	
	framebufferClose(&fb);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	/////////////////////////////// FIN INTRO //////////////////////////////////////////////////////
	
	
	
	Thread id_thread_playmodule;

	// init animations
	
	nombre_total_animation = sizeof(liste_animation) / sizeof(liste_animation[0]);
	
	numero_animation_en_cours = 0;
	pos_rotation_x = liste_animation[numero_animation_en_cours].angle_x_initial;
	pos_rotation_y = liste_animation[numero_animation_en_cours].angle_y_initial;
	pos_rotation_z = liste_animation[numero_animation_en_cours].angle_z_initial;
	increment_pos_rotation_x = liste_animation[numero_animation_en_cours].increment_angle_x;
	increment_pos_rotation_y = liste_animation[numero_animation_en_cours].increment_angle_y;
	increment_pos_rotation_z = liste_animation[numero_animation_en_cours].increment_angle_z;
	duree_animation_encours = liste_animation[numero_animation_en_cours].duree_animation;
	pos_x_cube_opengl = liste_animation[numero_animation_en_cours].posx_initial;
	pos_y_cube_opengl = liste_animation[numero_animation_en_cours].posy_initial;
	pos_z_cube_opengl = liste_animation[numero_animation_en_cours].posz_initial;
	
	
	appletSetCpuBoostMode(ApmCpuBoostMode_FastLoad);

	
	boink.nb_points = 146;
	boink.liste_des_points = &points[0];
	boink.liste_des_triangles = &faces_boink[0];
	boink.nb_faces = 288;
	
	// Set mesa configuration (useful for debugging)
    setMesaConfig();

    // Initialize EGL on the default window
    if (!initEgl(nwindowGetDefault()))
        return EXIT_FAILURE;

    // Load OpenGL routines using glad
    gladLoadGL();

    // Initialize our scene
    sceneInit();
    

	
    // Retrieve the default window
    //NWindow* win = nwindowGetDefault();

    // Create a linear double-buffered framebuffer
    //Framebuffer fb;
	//framebufferCreate(&fb, win, FB_WIDTH, FB_HEIGHT, PIXEL_FORMAT_RGBA_8888, 2);
    //framebufferMakeLinear(&fb);

	texture_screen_buffer = (u32*) malloc(buffer_texture_largeur * buffer_texture_hauteur * 4);
	memset(texture_screen_buffer , 0,buffer_texture_largeur * buffer_texture_hauteur * 4);
	memset(texture_screen_buffer+(boink_screen_largeur*boink_screen_hauteur*4) , 0xFF, boink_screen_largeur*boink_screen_hauteur*4);
	// texture_screen_buffer+(boink_screen_largeur*boink_screen_hauteur*4)
    
	
	    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);

	threadCreate(&id_thread_playmodule, remplissage_module, NULL, NULL, 0x8000, 0x30, 2);
    threadStart(&id_thread_playmodule);
	
	int nb_points = sizeof(points) / sizeof(points[0]);
	
	
	int anglexcourant = 0;
	int angleycourant = 0;
	int anglezcourant = 0;
	int	position_z_observateur=900;
	// 900
	#define min_zdep -200
	#define max_zdep 300
	int inc_zdep=1;
	
	objet_en_cours = &boink;
	
	precalc_sin_cos();
	
	int tailleface = sizeof(faces_boink)/(sizeof(faces_boink[0]));

	

	previousTick = armGetSystemTick();
	
	
	int tick=0;
	int sensx=1;
	int sensy=1;
	//float coscalc;
	
	//float posy=0;
	//float posyt=0;
	//float posyn=0;
	float posx=0;
	
	int angle_y=0;
	int inc_angle_y = 1;

	glBindVertexArray(s_vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
	
	//pos_rotation_x = 0;
	//pos_rotation_y = 0;
	//pos_rotation_z = 0;
	
	int nbfacesmaxi=0;
	
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_texture_largeur, buffer_texture_hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );

// init du fullscreen TCB
scrollers_fullscreen_TCB[0].pointeur_vers_debut_message = texte0;
scrollers_fullscreen_TCB[0].delai_avant_demarrage = 1359;
scrollers_fullscreen_TCB[0].pointeur_en_cours_dans_le_message = texte0;
scrollers_fullscreen_TCB[0].position_dans_le_caractere_actuel = 0;
scrollers_fullscreen_TCB[0].largeur_caractere_actuel = 0;

scrollers_fullscreen_TCB[1].pointeur_vers_debut_message = texte1;
scrollers_fullscreen_TCB[1].delai_avant_demarrage = 1139;
scrollers_fullscreen_TCB[1].pointeur_en_cours_dans_le_message = texte1;
scrollers_fullscreen_TCB[1].position_dans_le_caractere_actuel = 0;
scrollers_fullscreen_TCB[1].largeur_caractere_actuel = 0;

scrollers_fullscreen_TCB[2].pointeur_vers_debut_message = texte2;
scrollers_fullscreen_TCB[2].delai_avant_demarrage = 700;
scrollers_fullscreen_TCB[2].pointeur_en_cours_dans_le_message = texte2;
scrollers_fullscreen_TCB[2].position_dans_le_caractere_actuel = 0;
scrollers_fullscreen_TCB[2].largeur_caractere_actuel = 0;

scrollers_fullscreen_TCB[3].pointeur_vers_debut_message = texte3;
scrollers_fullscreen_TCB[3].delai_avant_demarrage = 70;
scrollers_fullscreen_TCB[3].pointeur_en_cours_dans_le_message = texte3;
scrollers_fullscreen_TCB[3].position_dans_le_caractere_actuel = 0;
scrollers_fullscreen_TCB[3].largeur_caractere_actuel = 0;

scrollers_fullscreen_TCB[4].pointeur_vers_debut_message = texte4;
scrollers_fullscreen_TCB[4].delai_avant_demarrage = 931;
scrollers_fullscreen_TCB[4].pointeur_en_cours_dans_le_message = texte4;
scrollers_fullscreen_TCB[4].position_dans_le_caractere_actuel = 0;
scrollers_fullscreen_TCB[4].largeur_caractere_actuel = 0;

scrollers_fullscreen_TCB[5].pointeur_vers_debut_message = texte5;
scrollers_fullscreen_TCB[5].delai_avant_demarrage = 1359;
scrollers_fullscreen_TCB[5].pointeur_en_cours_dans_le_message = texte5;
scrollers_fullscreen_TCB[5].position_dans_le_caractere_actuel = 0;
scrollers_fullscreen_TCB[5].largeur_caractere_actuel = 0;

scrollers_fullscreen_TCB[6].pointeur_vers_debut_message = texte6;
scrollers_fullscreen_TCB[6].delai_avant_demarrage = 1370;
scrollers_fullscreen_TCB[6].pointeur_en_cours_dans_le_message = texte6;
scrollers_fullscreen_TCB[6].position_dans_le_caractere_actuel = 0;
scrollers_fullscreen_TCB[6].largeur_caractere_actuel = 0;

scrollers_fullscreen_TCB[7].pointeur_vers_debut_message = texte7;
scrollers_fullscreen_TCB[7].delai_avant_demarrage = 1370;
scrollers_fullscreen_TCB[7].pointeur_en_cours_dans_le_message = texte7;
scrollers_fullscreen_TCB[7].position_dans_le_caractere_actuel = 0;
scrollers_fullscreen_TCB[7].largeur_caractere_actuel = 0;

	// stbi_set_flip_vertically_on_load(true);
	// img = stbi_load_from_memory((const stbi_uc*) title_png, title_png_size, &width, &height, &nchan, 4);
	//width = 8;
	//height = 40;

	back_rgba = (u32*) malloc(largeur_motif_fond_fullscreen_TCB*hauteur_motif_fond_fullscreen_TCB*4);

	u32 pixel;
	pos_source=0;
	int pos_destination=0;

	for (int i=0; i<largeur_motif_fond_fullscreen_TCB*hauteur_motif_fond_fullscreen_TCB;i++)
	{
		pixel = 0xFF000000 | back[pos_source+2] << 16 | back[pos_source+1] <<8 | back[pos_source+0]  ;
		// ABVR
		pos_source=pos_source+3;
		back_rgba[pos_destination]=pixel;
		pos_destination++;
	}




	// allocation des buffers graphiques
	// 2 ecrans de largeur * 4 exemplaires pour le parallax
	pointeur_buffer_ecrans_scrolling = (u32*) malloc( hauteur_fullscreen_TCB * largeur_fullscreen_TCB * 2 * nb_ecrans_fullscreen_TCB *4 );
	memset(pointeur_buffer_ecrans_scrolling,0, hauteur_fullscreen_TCB * largeur_fullscreen_TCB * 2 * nb_ecrans_fullscreen_TCB * 4);

	// des pointeurs sur tout cela
	//u32* pointeur_actuel_sur_buffer = pointeur_buffer_ecrans_scrolling;

	// position actuel dans l'ecran pour gestion du scrolling horizontal
	int position_actuelle_scrolling_horizontale_fullscreen_TCB = 0;
	// position actuelle sur 1 des 4 ecrans pour parallax
	int position_parallax_fullscreen_TCB=0;




	// decompression logo format png 
	img = stbi_load_from_memory((const stbi_uc*) title_png, title_png_size, &width, &height, &nchan, 4);
	/*for(int x=0;x<8;x++)
	{
		TRACE("%X\n",img[x]);
	}
	*/
	u32* pointeurimgu32 = (u32*) img;


	buffer_logo = (u32*) malloc( hauteur_logo_fullscreen_TCB * largeur_fullscreen_TCB * 4);
	memset(buffer_logo, 0,hauteur_logo_fullscreen_TCB * largeur_fullscreen_TCB * 4);
	pos_source=0;
	pos_dest =70;
	for(int y=0;y<hauteur_logo_fullscreen_TCB;y++)
	{
		for(int x=0;x<largeur_logo_fullscreen_TCB;x++)
		{
			// B V R
			u32 pixel= pointeurimgu32[pos_source+0];
			buffer_logo[pos_dest]=pixel;
			pos_source++;
			pos_dest++;
		}
		pos_dest=pos_dest + largeur_fullscreen_TCB - largeur_logo_fullscreen_TCB;
	}

	stbi_image_free(img);




	// remmplissage avec le parallax a partir de la ligne ligne_debut_remplissage_fond sur les 8 ecrans

	pos_dest = ligne_debut_remplissage_fond * largeur_fullscreen_TCB*2;
	pos_source=0;
	
	//TRACE("hauteur_fullscreen_TCB  : %d\n", (hauteur_fullscreen_TCB ));
	//TRACE(" ligne_debut_remplissage_fond : %d\n", (ligne_debut_remplissage_fond));
	//TRACE("hauteur_fullscreen_TCB - ligne_debut_remplissage_fond : %d\n", (hauteur_fullscreen_TCB - ligne_debut_remplissage_fond));
	
	
	for(int y=0;y< (hauteur_fullscreen_TCB - ligne_debut_remplissage_fond) ;y++)
	{
		for(int x=0;x< largeur_fullscreen_TCB*2;x++)
		{
			// ecran 1 parallax
			pos_source = (x % largeur_motif_fond_fullscreen_TCB ) + ((y % hauteur_motif_fond_fullscreen_TCB )*largeur_motif_fond_fullscreen_TCB);
			pixel = back_rgba[pos_source];
			pointeur_buffer_ecrans_scrolling[pos_dest]=pixel;
			// ecran 2 parallax
			pos_source = ((x-1+8) % largeur_motif_fond_fullscreen_TCB  ) + ((y % hauteur_motif_fond_fullscreen_TCB )*largeur_motif_fond_fullscreen_TCB);
			pixel = back_rgba[pos_source];
			pointeur_buffer_ecrans_scrolling[pos_dest+largeur_fullscreen_TCB*2*1* hauteur_fullscreen_TCB]=pixel;
			// ecran 3 parallax
			pos_source = ((x-2+8) % largeur_motif_fond_fullscreen_TCB  ) + ((y % hauteur_motif_fond_fullscreen_TCB )*largeur_motif_fond_fullscreen_TCB);
			pixel = back_rgba[pos_source];
			pointeur_buffer_ecrans_scrolling[pos_dest+largeur_fullscreen_TCB*2*2* hauteur_fullscreen_TCB]=pixel;
			// ecran 4 parallax
			pos_source = ((x-3+8) % largeur_motif_fond_fullscreen_TCB  ) + ((y % hauteur_motif_fond_fullscreen_TCB )*largeur_motif_fond_fullscreen_TCB);
			pixel = back_rgba[pos_source];
			pointeur_buffer_ecrans_scrolling[pos_dest+largeur_fullscreen_TCB*2*3* hauteur_fullscreen_TCB]=pixel;
			// ecran 5 parallax
			pos_source = ((x-4+8) % largeur_motif_fond_fullscreen_TCB  ) + ((y % hauteur_motif_fond_fullscreen_TCB )*largeur_motif_fond_fullscreen_TCB);
			pixel = back_rgba[pos_source];
			pointeur_buffer_ecrans_scrolling[pos_dest+largeur_fullscreen_TCB*2*4* hauteur_fullscreen_TCB]=pixel;
			// ecran 6 parallax
			pos_source = ((x-5+8) % largeur_motif_fond_fullscreen_TCB  ) + ((y % hauteur_motif_fond_fullscreen_TCB )*largeur_motif_fond_fullscreen_TCB);
			pixel = back_rgba[pos_source];
			pointeur_buffer_ecrans_scrolling[pos_dest+largeur_fullscreen_TCB*2*5* hauteur_fullscreen_TCB]=pixel;
			// ecran 7 parallax
			pos_source = ((x-6+8) % largeur_motif_fond_fullscreen_TCB  ) + ((y % hauteur_motif_fond_fullscreen_TCB )*largeur_motif_fond_fullscreen_TCB);
			pixel = back_rgba[pos_source];
			pointeur_buffer_ecrans_scrolling[pos_dest+largeur_fullscreen_TCB*2*6* hauteur_fullscreen_TCB]=pixel;
			// ecran 8 parallax
			pos_source = ((x-7+8) % largeur_motif_fond_fullscreen_TCB  ) + ((y % hauteur_motif_fond_fullscreen_TCB )*largeur_motif_fond_fullscreen_TCB);
			pixel = back_rgba[pos_source];
			pointeur_buffer_ecrans_scrolling[pos_dest+largeur_fullscreen_TCB*2*7* hauteur_fullscreen_TCB]=pixel;



			pos_dest=pos_dest + 1;
		}
		//pos_dest=pos_dest + largeur_fullscreen_TCB;
		
	}
	
	




	// début init sprites
	img = stbi_load_from_memory((const stbi_uc*) sprite_png, sprite_png_size, &width, &height, &nchan, 4);
	
	// allouer 9 buffers de sprite
		for (int x=0;x<nombre_sprites_fullscreen_TCB;x++)
		{
			sprites[x].dessin_sprite = (u32*) malloc(largeur_sprite_fullscreen_TCB*hauteur_sprite_fullscreen_TCB*4);
			memset(sprites[x].dessin_sprite,0,largeur_sprite_fullscreen_TCB*hauteur_sprite_fullscreen_TCB*4);
		}
		// allouer 8 buffers de CLS par sprite
		for (int x=0;x<nombre_sprites_fullscreen_TCB;x++)
		{
			//for (int nbecrans=0;nbecrans<nb_ecrans_fullscreen_TCB; nbecrans++)
			//{
				sprites[x].buffers_cls = (u32*) malloc(largeur_sprite_fullscreen_TCB*hauteur_sprite_fullscreen_TCB*4);
				memset(sprites[x].buffers_cls,0,largeur_sprite_fullscreen_TCB*hauteur_sprite_fullscreen_TCB*4);
			//}
		}

	// copier du RGBA aux buffers
	// source RGBA sprite = img
		pointeurimgu32 = (u32*) img;
		pos_source = 0;


		for (int numero_sprite=nombre_sprites_fullscreen_TCB-1;numero_sprite>=0;numero_sprite--)
		{

			pos_dest = 0;
			u32* pointeur_dest = sprites[numero_sprite].dessin_sprite;
			for (int y=0;y<hauteur_sprite_fullscreen_TCB;y++)
			{
				for(int x=0;x<largeur_sprite_fullscreen_TCB;x++)
				{
					u32 pixel= pointeurimgu32[pos_source]  ;
					pos_source++;
					pointeur_dest[pos_dest] = pixel;
					pos_dest++;
				}

			}
		}

	// initialiser	les buffers et les pointeurs de sauvegarde de CLS
		u32* pointeur_generique_CLS;
		pointeur_generique_CLS = pointeur_buffer_ecrans_scrolling  + 100 * largeur_fullscreen_TCB;
		for (int num_sprite=0;num_sprite<nombre_sprites_fullscreen_TCB;num_sprite++)
		{
			//for (int num_ecran=0;num_ecran<nb_ecrans_fullscreen_TCB;num_ecran++)
			//{
				sprites[num_sprite].pointeur_destination_precedents = pointeur_generique_CLS;
				u32* pointeur_buffer_sauvegarde_CLS = sprites[num_sprite].buffers_cls;
				u32* pointeur_source = pointeur_generique_CLS;
				u32* pointeur_dest = sprites[num_sprite].buffers_cls;
				for(int y=0;y<hauteur_sprite_fullscreen_TCB;y++)
				{
					for(int x=0;x<largeur_sprite_fullscreen_TCB;x++)
					{
						pointeur_dest[0] = pointeur_source[0];
						pointeur_dest++;
						pointeur_source++;
					}
				pointeur_source = pointeur_source  + largeur_fullscreen_TCB*2 - largeur_sprite_fullscreen_TCB;
				}
			//}
		}

	stbi_image_free(img);

	// fin init sprites
	//TRACE("fini Init Sprites : \n");

	// init scrollers
	fontsimg = stbi_load_from_memory((const stbi_uc*) font_png, font_png_size, &width, &height, &nchan, 4);
	fonts = (u32*) fontsimg;
	
	
	
	/*
	fonts
	420 x 240
	hauteur d'un caractere = 40
	*/
	

	position_parallax_fullscreen_TCB =0;
	iteration = 0;
	pos_scrolling_horizontal_dans_ecran = 0;


// ----------------------------------------------------------------
// init sprites TCB
// ----------------------------------------------------------------
	// le buffer d'ecran de 320x200
	buffer_ecran = (u32*) malloc(largeur_sprites_TCB*hauteur_sprites_TCB*4);
	memset(buffer_ecran, 0, largeur_sprites_TCB*hauteur_sprites_TCB*4);
	
	// on decompresse le fond
	fond_png = stbi_load_from_memory((const stbi_uc*) bg2_png, bg2_png_size, &width, &height, &nchan, 4);
	
	
	// un buffer pour le fond
	fond_ecran = (u32*) malloc(largeur_sprites_TCB*hauteur_sprites_TCB*4);
	memset(fond_ecran, 0, largeur_sprites_TCB*hauteur_sprites_TCB*4);
	
	// on copie le png dans le buffer de fond d'ecran
	u32* pointeurbgu32 = (u32*)fond_png ;
	pos_source=0;
	pos_dest =0;
	for(int y=0;y<hauteur_sprites_TCB;y++)
	{
		for(int x=0;x<largeur_sprites_TCB;x++)
		{
			
			fond_ecran[pos_dest]=pointeurbgu32[pos_source];
			pos_source++;
			pos_dest++;
		}
		
	}

	// on libere la RAM
	stbi_image_free(fond_png);


	// la boule
	// un buffer pour la boule
	ball = (u32*) malloc(largeur_ball_sprites_TCB*hauteur_ball_sprites_TCB*4);
	memset(fond_ecran, 0, largeur_ball_sprites_TCB*hauteur_ball_sprites_TCB*4);

	// on decompresse la ball
	stbiball_png = stbi_load_from_memory((const stbi_uc*) ball2_png, ball2_png_size, &width, &height, &nchan, 4);
	
	
	// on copie le ball png dans le buffer ball
	pointeurbgu32 = (u32*)stbiball_png ;
	pos_source=0;
	pos_dest =0;
	for(int y=0;y<hauteur_ball_sprites_TCB;y++)
	{
		for(int x=0;x<largeur_ball_sprites_TCB;x++)
		{
			
			ball[pos_dest]=pointeurbgu32[pos_source];
			pos_source++;
			pos_dest++;
		}
		
	}


	// on libere la RAM
	stbi_image_free(stbiball_png);

	initcossin();
	
	int pxa1=0; 
	int pxa2=0; 
	int pya1=0; 
	int pya2=0; 
	
///////////////////////////////////////////////////
// debut init megascroll ULM
///////////////////////////////////////////////////
	// le buffer d'ecran de 384 x 264
	buffer_ecran__avec_marges_megascroll_ULM = (u32*) malloc(largeur_megascroll_ULM*(hauteur_megascroll_ULM+80)*4);
	buffer_ecran_megascroll_ULM = buffer_ecran__avec_marges_megascroll_ULM + ( 40* largeur_megascroll_ULM );
	memset(buffer_ecran_megascroll_ULM , 0, largeur_megascroll_ULM*hauteur_megascroll_ULM*4);
	
	
	// on decompresse la tile
	tile_megascroll_ULM_png = stbi_load_from_memory((const stbi_uc*) tile88_png, tile88_png_size, &width, &height, &nchan, 4);
	
	pointeurtileu32 = (u32*)tile_megascroll_ULM_png ;
	
	// on crée un buffer de hauteur 8 lignes, et de largeur 384 + 8 *2
	ligne_tile_megascroll_ULM = (u32*) malloc( (largeur_megascroll_ULM + largeur_tile_megascroll_ULM)*hauteur_tile_megascroll_ULM*2*4);
	
	// on copie sur 384+8 pixels de large, les 8 lignes
	pos_dest = 0;
	for (int y=0;y<hauteur_tile_megascroll_ULM;y++)
	{
		for (int x=0;x<(largeur_megascroll_ULM + largeur_tile_megascroll_ULM);x++)
		{
			ligne_tile_megascroll_ULM[pos_dest] = pointeurtileu32[(x % largeur_tile_megascroll_ULM)+(y * largeur_tile_megascroll_ULM)];
			pos_dest++;
		}
	}
	for (int y=0;y<hauteur_tile_megascroll_ULM;y++)
	{
		for (int x=0;x<(largeur_megascroll_ULM + largeur_tile_megascroll_ULM);x++)
		{
			ligne_tile_megascroll_ULM[pos_dest] = pointeurtileu32[(x % largeur_tile_megascroll_ULM)+(y * largeur_tile_megascroll_ULM)];
			pos_dest++;
		}
	}
	
	// la font
	font_megascroll_ULM_png = stbi_load_from_memory((const stbi_uc*) fontULM_png, fontULM_png_size, &width, &height, &nchan, 4);
	
	pointeur_fnt_u32_megascroller_ULM = (u32*) font_megascroll_ULM_png;
	
	
	// la boule
	ball_megascroll_ULM_png = stbi_load_from_memory((const stbi_uc*) ball1616_png, ball1616_png_size, &width, &height, &nchan, 4);
	
	
	
	pointeur_ball_u32_megascroller_ULM = (u32*) ball_megascroll_ULM_png;


///////////////////////////////////////////////////
// debut init TechTech
///////////////////////////////////////////////////

	// le buffer d'ecran de 320 x 200
	buffer_ecran_TechTech = (u32*) malloc(largeur_TechTech*(hauteur_TechTech)*4);
	memset(buffer_ecran_TechTech , 0xFF, largeur_TechTech*hauteur_TechTech*4);
	
	// decompresse fond rose
	pointeur_fond_rose_TechTech = stbi_load_from_memory((const stbi_uc*) Grodan_pink_png, Grodan_pink_png_size, &width, &height, &nchan, 4);
	pointeur_fond_rose_TechTech_u32 = (u32*) pointeur_fond_rose_TechTech;
	
	// decompresse fond vert
	pointeur_fond_vert_TechTech = stbi_load_from_memory((const stbi_uc*) Grodan_green_png, Grodan_green_png_size, &width, &height, &nchan, 4);
	pointeur_fond_vert_TechTech_u32 = (u32*) pointeur_fond_vert_TechTech;
	
	// decompresse sprites
	pointeur_sprites_TechTech = stbi_load_from_memory((const stbi_uc*) spriteTECH_png, spriteTECH_png_size, &width, &height, &nchan, 4);
	pointeur_sprites_TechTech_u32 = (u32*) pointeur_sprites_TechTech;
	
	// decompresse les 2 PNGs fonte et fond
	//  fontes bas : 24 large, 32 de haut, espacée , aux lignes x33, : lignes : 0,33,66 etc
	// multiplier la police par 3
	// integrer le fond qui fait 320 x 96
	
	pointeur_fonte_scrolling_bas_TechTech = stbi_load_from_memory((const stbi_uc*) bsfont9696_png, bsfont9696_png_size, &width, &height, &nchan, 4);
	pointeur_fonte_scrolling_bas_TechTech_u32 = (u32*) pointeur_fonte_scrolling_bas_TechTech;
	
	pointeur_rasters_scrolling_bas_TechTech = stbi_load_from_memory((const stbi_uc*) bigscrollraster_png, bigscrollraster_png_size, &width, &height, &nchan, 4);
	pointeur_rasters_scrolling_bas_TechTech_u32 = (u32*) pointeur_rasters_scrolling_bas_TechTech;
	


	
	
	// build de la police en integrant les rasters
	// 6 lignes de 32 de haut, x10 caracteres de 
	// (10 x 96)  x 96 x 6 lignes = 
	// fonte finale : 96 x 96
	// x 4 en largeur, x3 en hauteur
	
	u32 couleur_fond_scrollingbas_TechTech ,couleur_fonte_scrollingbas_TechTech ;
	
	for (int y=0;y<600;y++)
	{
		for (int x=0;x<960;x++)
		{
			couleur_fonte_scrollingbas_TechTech = pointeur_fonte_scrolling_bas_TechTech_u32[(y*960)+x];
			couleur_fond_scrollingbas_TechTech =  pointeur_rasters_scrolling_bas_TechTech_u32[( y % 99 ) * 320 ];
			couleur_fond_scrollingbas_TechTech = couleur_fond_scrollingbas_TechTech & couleur_fonte_scrollingbas_TechTech;
			pointeur_fonte_scrolling_bas_TechTech_u32[(y*960)+x] = couleur_fond_scrollingbas_TechTech;
		}
	}
		
	// scrolling bas 1er caractere
	// pour commencer par un caractere non vide
	// pointeur_pos_caractere_en_cours_scrolling_bas_TechTech = pointeur_fonte_scrolling_bas_TechTech_u32 + (960*99*2);
	pointeur_pos_caractere_en_cours_scrolling_bas_TechTech = pointeur_fonte_scrolling_bas_TechTech_u32 ;
	pos_dans_fonte_scrolling_bas_TechTech = 0;
	
		
	// fonte caractere scrolling verticaux
	pointeur_fonte_scrolling_vertical_TechTech = stbi_load_from_memory((const stbi_uc*) fontverticale1615_png, fontverticale1615_png_size, &width, &height, &nchan, 4);
	pointeur_fonte_scrolling_vertical_TechTech_u32 = (u32*) pointeur_fonte_scrolling_vertical_TechTech;
	
	//pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech = pointeur_fonte_scrolling_vertical_TechTech_u32 + (330*29*4);
	pointeur_pos_caractere_en_cours_scrolling_vertical_TechTech = pointeur_fonte_scrolling_vertical_TechTech_u32 ;
	pos_dans_fonte_scrolling_vertical_TechTech = 0;

	
	
	// charger police 8x8
	pointeur_fonte_scrolling_8x8_TechTech = stbi_load_from_memory((const stbi_uc*) lfont_png, lfont_png_size, &width, &height, &nchan, 4);
	pointeur_fonte_scrolling_8x8_TechTech_u32 = (u32*) pointeur_fonte_scrolling_8x8_TechTech;
	
	pointeur_pos_caractere_en_cours_scrolling_8x8h_TechTech = pointeur_fonte_scrolling_8x8_TechTech_u32 ;
	pos_dans_fonte_scrolling_8x8h_TechTech = 0;
	pointeur_pos_caractere_en_cours_scrolling_8x8b_TechTech = pointeur_fonte_scrolling_8x8_TechTech_u32 ;
	pos_dans_fonte_scrolling_8x8b_TechTech = 0;


///////////////////////////////////////////////////
// debut init Shadow of the Beast
///////////////////////////////////////////////////

	// le buffer d'ecran de 320 x 200
	buffer_ecran_Shadow = (u32*) malloc(largeur_Shadow*(hauteur_Shadow+30)*4);
	memset(buffer_ecran_Shadow , 0x00, largeur_Shadow*(hauteur_Shadow+30)*4);
	
	// decompresse Overlay
	pointeur_overlay_Shadow = stbi_load_from_memory((const stbi_uc*) overlayshadow_png, overlayshadow_png_size, &width, &height, &nchan, 4);
	pointeur_overlay_Shadow_u32 = (u32*) pointeur_overlay_Shadow;
	
	pointeur_table_overlay_Shadow_u32 = (u32*) tableoverlay;
	
	
	
	// transformation de la table
	for (int i = 0;i<1411;i++)
	{
		u32 valeur_overlay_Shadow = pointeur_table_overlay_Shadow_u32[i];
		valeur_overlay_Shadow = (valeur_overlay_Shadow >> 24) + (((valeur_overlay_Shadow>>16) & 0xFF) << 8) + (((valeur_overlay_Shadow>>8) & 0xFF) << 16);
		valeur_overlay_Shadow = valeur_overlay_Shadow / 230;
		pointeur_table_overlay_Shadow_u32[i] = (u32) valeur_overlay_Shadow;
		// TRACE("valeur_overlay_Shadow : %X %X\n",valeur_overlay_Shadow,pointeur_table_overlay_Shadow_u32[i] );
	}

	
	
	
	// decompresse Background
	pointeur_background_Shadow = stbi_load_from_memory((const stbi_uc*) backgroundshadow_png, backgroundshadow_png_size, &width, &height, &nchan, 4);
	pointeur_background_Shadow_u32 = (u32*) pointeur_background_Shadow;
	
	
	// decompresse sprites
	pointeur_sprites_Shadow = stbi_load_from_memory((const stbi_uc*) spritesshadow_png, spritesshadow_png_size, &width, &height, &nchan, 4);
	pointeur_sprites_Shadow_u32 = (u32*) pointeur_sprites_Shadow;
	
	// decompresse sprites Amiga
	pointeur_Amiga_Shadow = stbi_load_from_memory((const stbi_uc*) SotB_Rip_Amiga_1_png, SotB_Rip_Amiga_1_png_size, &width, &height, &nchan, 4);
	pointeur_Amiga_Shadow_u32 =(u32*)  pointeur_Amiga_Shadow;


	luminosite=0.0f;
// ------------ main loop ----------------------------------------------------------------------

	initialTick = armGetSystemTick();
	while (appletMainLoop())
    {
		previousTick = armGetSystemTick();
		
        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) break; // break in order to return to hbmenu

		// passage en mode manuel
		if (kDown) 
		{
			mode_manuel=1;
			attente_demarrage_fullscreen_TCB=0;
		}

		#define increment_mouvements 0.005f
		#define increment_deplacement 0.02f

		if (kDown & KEY_X)
        {
			pos_rotation_x=0;
			pos_rotation_y=0;
			pos_rotation_z=MONPI;
			increment_pos_rotation_x=0.0f;
			increment_pos_rotation_y=0.0f;
			increment_pos_rotation_z=0.0f;
			pos_x_cube_opengl = 0.0f;
			pos_y_cube_opengl = 0.0f;
			pos_z_cube_opengl = -2.0f;
			
		}
		if (kDown & KEY_RSTICK_RIGHT) increment_pos_rotation_y=increment_pos_rotation_y+increment_mouvements;
		if (kDown & KEY_RSTICK_LEFT) increment_pos_rotation_y=increment_pos_rotation_y-increment_mouvements;
		if (kDown & KEY_RSTICK_UP) increment_pos_rotation_x=increment_pos_rotation_x+increment_mouvements;
        if (kDown & KEY_RSTICK_DOWN) increment_pos_rotation_x=increment_pos_rotation_x-increment_mouvements;
        
        if (kDown & KEY_DLEFT) increment_pos_rotation_z=increment_pos_rotation_z+increment_mouvements;
        if (kDown & KEY_DRIGHT) increment_pos_rotation_z=increment_pos_rotation_z-increment_mouvements;

		if (kDown & KEY_LSTICK_RIGHT) pos_x_cube_opengl = pos_x_cube_opengl+increment_deplacement;
		if (kDown & KEY_LSTICK_LEFT) pos_x_cube_opengl = pos_x_cube_opengl-increment_deplacement;
		if (kDown & KEY_LSTICK_UP) pos_y_cube_opengl = pos_y_cube_opengl+increment_deplacement;
		if (kDown & KEY_LSTICK_DOWN) pos_y_cube_opengl = pos_y_cube_opengl-increment_deplacement;
		
		if (kDown & KEY_DUP) pos_z_cube_opengl = pos_z_cube_opengl-increment_deplacement;
        if (kDown & KEY_DDOWN) pos_z_cube_opengl = pos_z_cube_opengl+increment_deplacement;


		//if (kDown & KEY_DUP)	Zdep=Zdep+10;
         //       if (kDown & KEY_DDOWN) Zdep=Zdep-10;

		//if (kDown & KEY_LSTICK_RIGHT) Xdep=Xdep+10;
		//if (kDown & KEY_LSTICK_LEFT) Xdep=Xdep-10;

		//if (kDown & KEY_LSTICK_DOWN) Ydep=Ydep+10;
        //        if (kDown & KEY_LSTICK_UP) Ydep=Ydep-10;
	
 
		


	


		// Retrieve the framebuffer
        // framebuf = (u32*) framebufferBegin(&fb, &stride);

		
		
		// mouvements divers
		tick++;
		//int coscalc2 =  cos(tick);
		//coscalc = abs(coscalc2);
		incangley = (10*sensy);
		incanglex = (2*sensx);
		
		
		Zdep = Zdep + inc_zdep;
		if (Zdep > max_zdep || Zdep < min_zdep) 
		{ 
			inc_zdep = - inc_zdep;
		}
		
		// pos en y1
		// sur 720, sinusoide entre -320 et 320
		posy2D = (sin(angle_y*(MONPI/180))* (400/2));
		if (posy2D> 100) inc_angle_y = -inc_angle_y;
		if (posy2D < -200) inc_angle_y = 1;
			
		angle_y = angle_y+inc_angle_y;
		if (angle_y == 360)
		{
			angle_y=0;
		}
		if (angle_y == 360)	inc_angle_y = 1;


		Xdep = posx;
		// Ydep = posyt;

		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageavantCLS = (toto / TICKS_PER_FRAME)*100;



		
		// clear screen
		efface_tous_les_ecrans();
								
		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageCLS = (toto / TICKS_PER_FRAME)*100;


		// dessine le quadrillage
		//
		int width=boink_screen_largeur;
		int height=boink_screen_hauteur; 
		int sx=40;
		int sy=40;
		#define couleur_grille 0xFFFFFF00
		
		// lignes horizontales
		// nb lignes
		int nb_cases = height / sy;
		int posx_quadrillage = - ( width / 2);
		int posy_quadrillage = - (height/2);
		int max_x_quadrillage = ( width / 2);
		int max_y_quadrillage = ( height / 2);
		
		for (int i=0;i<nb_cases ;i++) 
			{
				draw_line(posx_quadrillage,posy_quadrillage,max_x_quadrillage, posy_quadrillage, couleur_grille);
				posy_quadrillage = posy_quadrillage + sy;
			}
			
		// lignes verticales
		// 
		
		posx_quadrillage = - ( width / 2);
		posy_quadrillage = - (height/2);
		max_x_quadrillage = ( width / 2);
		max_y_quadrillage = ( height / 2);
		// 720/2 = 360 
		int sol_y = 140;
		nb_cases =  width/ sx;
		for (int i=0;i<nb_cases;i++) 
		{
			draw_line(posx_quadrillage,posy_quadrillage,posx_quadrillage, sol_y, couleur_grille);
			posx_quadrillage = posx_quadrillage + sx;
		}
		
		nb_cases =  width/ sx;
		posx_quadrillage = - ( width / 2);
		int posx_quadrillage_bas = -width;
		for (int i=0;i<nb_cases;i++)
		{
			draw_line(posx_quadrillage,sol_y,posx_quadrillage_bas, max_y_quadrillage, couleur_grille);
			posx_quadrillage = posx_quadrillage + sx;
			posx_quadrillage_bas = ((((i*sx)-(width*0.5))/sx)*width)*0.1;;
		}
		
		

		// calcul constantes suivant angles
		calc_valeurs_globales( anglexcourant, angleycourant, anglezcourant);


        anglexcourant = anglexcourant + incanglex;
		angleycourant = angleycourant + incangley;
		anglezcourant = anglezcourant + incanglez;
		if (anglexcourant<0)
                        { anglexcourant=3599;}
                if (angleycourant<0)
                        { angleycourant=3599;}
                if (anglezcourant<0)
                        { anglezcourant=3599;}
                if (anglexcourant>3599)
                        { anglexcourant=0;}
		if (angleycourant>3599)
                        { angleycourant=0;}
		if (anglezcourant>3599)
                        { anglezcourant=0;}

		// rotation et projection des points
		calc_point(position_z_observateur, objet_en_cours->nb_points, objet_en_cours->liste_des_points);

		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageCalc = (toto / TICKS_PER_FRAME)*100;

		tri_faces(objet_en_cours);

		// on a les faces triées dans faces_triees
	
		struct struct_triangle *liste_faces = objet_en_cours->liste_des_triangles;

		for(int cx=0;cx<boink.nb_faces;cx++)
		{
			int numero_face = faces_triees[cx].numero_face;
			int pr1 = liste_faces[numero_face].p1;
			int pr2 = liste_faces[numero_face].p2;
			int pr3 = liste_faces[numero_face].p3;
			u32 couleurface = liste_faces[numero_face].color;
			
			// calcul de la visibilite
			// https://github.com/pchalamet/amiga-sources/blob/master/Sources/Demos/Convex_Vektors/3d_Cube.s
			//int    xv1,yv1,xv2,yv2,xv3,yv3;
			//int     calcul1,calcul2;

			//xv1 = points[pr1].x2D;
			//yv1 = points[pr1].y2D;
			//xv2 = points[pr2].x2D;
			//yv2 = points[pr2].y2D;
			//xv3 = points[pr3].x2D;
			//yv3 = points[pr3].y2D;
			//calcul1 = ((xv2-xv1)*(yv3-yv1));
			//calcul2 = ((xv3-xv1)*(yv2-yv1));
			// test de visibilite
			//if (calcul2>=calcul1)
            //    {}
			//else
			//{

				draw_triangle(pr1, pr2, pr3, couleurface  );
			//}
		
			//draw_line(points[pr1].x2D,points[pr1].y2D,points[pr2].x2D,points[pr2].y2D, couleurface  );
			//draw_line(points[pr2].x2D,points[pr2].y2D,points[pr3].x2D,points[pr3].y2D, couleurface );
			//draw_line(points[pr3].x2D,points[pr3].y2D,points[pr1].x2D,points[pr1].y2D, couleurface );
		}


///////////////////////////////////////////////////////////// FIN de la partie BOINK


		///////////////////////////////////////////////////
		// on met a jour le scroller
		///////////////////////////////////////////////////

		attente_demarrage_fullscreen_TCB--;
		if (attente_demarrage_fullscreen_TCB<0)
		{
			display_scrollers(position_parallax_fullscreen_TCB);

			///////////////////////////////////////////////////
			// on avance les compteurs
			///////////////////////////////////////////////////

			pos_scrolling_horizontal_dans_ecran = pos_scrolling_horizontal_dans_ecran + nb_pixels_a_faire_entrer_par_vbl_vlb_fullscreen_TCB;
			if (pos_scrolling_horizontal_dans_ecran == largeur_fullscreen_TCB) pos_scrolling_horizontal_dans_ecran =0;
		
			position_parallax_fullscreen_TCB = ( position_parallax_fullscreen_TCB + 1);
			if (position_parallax_fullscreen_TCB==nb_ecrans_fullscreen_TCB) 
			{
				position_parallax_fullscreen_TCB=0;
			}
			iteration++;

		}
		///////////////////////////////////////////////////
		// Sprites
		///////////////////////////////////////////////////
		
		// tout effacer avant de tout afficher
		for (int s = 0 ; s < nombre_sprites_fullscreen_TCB ; s++)
		{
			efface_sprite_fullscreen_TCB(position_parallax_fullscreen_TCB,s);
		}

		float midx = (largeur_fullscreen_TCB - 18) / 2;
		float midy = 24 + (hauteur_fullscreen_TCB - 34) / 2;
		float incy = (hauteur_fullscreen_TCB - 38) / 4;
		//TRACE("midx, midy, incy : %f, %f,  %f \n",midx, midy ,incy);

		for (int s = 0 ; s < nombre_sprites_fullscreen_TCB ; s++)
		{
			float nit = iteration + (s * 5);
			float spx2 = midx + (midx * sin(nit / 25) * cos(nit / 300));
			float spy2 = midy + incy * sin(nit / 37) + (incy * cos(nit / 17));
			int spx = (int) spx2;
			int spy = (int) spy2;
			//TRACE("nit, spx, spy : %f, %d,  %d \n",nit, spx ,spy);
			sprites[s].x_en_cours = spx;
			sprites[s].y_en_cours = spy;
		}

		// tout sauvegarder avant de tout afficher
		for (int s = 0 ; s < nombre_sprites_fullscreen_TCB ; s++)
		{
			sauvegarde_sprite_fullscreen_TCB(position_parallax_fullscreen_TCB,s);
		}

		for (int s = 0 ; s < nombre_sprites_fullscreen_TCB ; s++)
		{
			affiche_sprite_fullscreen_TCB(s);
		}
	



////////////////////////////////////////////////////// FIN DE la partie Fullscreen TCB

///////////////////////////////////////////////////// debut sprites TCB

	
		// on copie le fond sur l'ecran
		cls_sprites_TCB();
	
		// affiche sprites
		
		int pxb1 = pxa1; 
		int pxb2 = pxa2; 
		int pyb1 = pya1; 
		int pyb2 = pya2; 
		
		for(int i=0;i<ballrec;i++)
		{
			int mod1 = mod(pxb1 , PILEN);
			int mod2 = mod(pxb2 , PILEN);
			float x1 = (RX1 * cosTab[mod1] + RX2 * sinTab[mod2]);
			mod1 = mod(pyb1,PILEN);
			mod2 = mod(pyb2,PILEN);
			float y1 = (RY1 * cosTab[mod1] + RY2 * sinTab[mod2]);
			int x = (int) x1;
			int y = (int) y1;
			x = x >> 15;
			y = y >> 15;
			
			x += (RESX-16)/2; 
     		y += (RESY-16)/2; 
			
			affiche_sprite_1616(x,y);
			
			pxb1 = pxb1 + 7*2; 
    		pxb2 = pxb2 - 4*2; 
    		pyb1 = pyb1 + 6*2; 
    		pyb2 = pyb2 - 3*2;
			
		}

		
		pxa1 += 3*2; 
		pxa2 += 2*2; 
		pya1 += -1*2; 
		pya2 += 2*2; 
		
		
//////////////////////////////// Fin sprites TCB ///////////////////////


///////////////////////////////////////////////////// // les routines du megascroller
		
		copie_fond();

		affiche_megascroll_ULM();
		avance_scroller();

		decalage_en_cours_horizontal_megascroll_ULM = decalage_en_cours_horizontal_megascroll_ULM + 2;
		if (decalage_en_cours_horizontal_megascroll_ULM > (courbe_horizontale_ULM2w_size-2)/5) decalage_en_cours_horizontal_megascroll_ULM =0;
	
		pointeur_pos_table_2_valeurs_w = pointeur_pos_table_2_valeurs_w + 4;
		// on avance de 2 u16 = 4 octets
		if (pointeur_pos_table_2_valeurs_w >= table_2_valeurs_w_size-4) pointeur_pos_table_2_valeurs_w=0;

		radian_mouvement_fond_vertical_megascroller_ULM = radian_mouvement_fond_vertical_megascroller_ULM + 0.01;


///////////////////////////////////////////////////// // FIN des routines du megascroller

///////////////////////////////////////////////////// debut routines TechTech

		if (flag_c==0)
		{
			// CLS de l'ecran
			memset(buffer_ecran_TechTech , 0x00, largeur_TechTech*hauteur_TechTech*4);
		}
		else
		{
			// copie fond vert
			copie_fond_vert_TechTech(x_fond_vert_TechTech,y_fond_vert_TechTech);
		}
		
		// copie fond rose
		copie_fond_rose_TechTech(x_fond_rose_TechTech,y_fond_rose_TechTech);
		
		// 2 petits scrollings sous scrolling vertical
		
		if (flag_d==1)
		{
			// scrolling vertical sous scrolling du bas
			affiche_scrolling_vertical_TechTech();
			avance_scrolling_vertical_TechTech();
		
		
			affiche_scrolling_8x8h_TechTech();
			avance_scrolling_8x8h_TechTech();
		
			// affiche scrolling du bas en raster orange 96 de haut
			affiche_scrolling_bas_TechTech();
			avance_scrolling_bas_TechTech();
		}
		
		affiche_scrolling_8x8b_TechTech();
		avance_scrolling_8x8b_TechTech();
		
		// affichage sprites ( en dernier, par dessus tout )
		// affiche de 11 vers 0
		if (flag_a==1) affiche_tous_les_sprites_TechTech();


		if (flag_b==1) deplace_fonds_Tech_Tech();


///////////////////////////////////////////////////// FIN routines TechTech

///////////////////////////////////////////////////// routines Shadow

		// CLS du bas
		memset(buffer_ecran_Shadow+(249 * largeur_Shadow) , 0, largeur_Shadow*(270-249)*4);


		for (int i=0;i<nombre_parallax_Shadow;i++) affiche_parallax_Shadow(i);

		// les sprites Amiga		
		
		// random numbers pour affichage arbres amiga
		int randomNumber = (rand() % 600) + 1;

		if (randomNumber > 5 and randomNumber <15 and flag_afficher_arbre_gauche_Amiga_shadow==0) 
		{
			//TRACE("randomNumber : %d\n",randomNumber);
			flag_afficher_arbre_gauche_Amiga_shadow=1;
			pos_x_dest_sprite_arbre_gauche_Amiga_Shadow = bord_droit_sprites_Amiga_Shadow;
		}

		if (randomNumber > 25 and randomNumber <35 and flag_afficher_arbre_droite_Amiga_shadow==0) 
		{
			//TRACE("randomNumber droite: %d\n",randomNumber);
			flag_afficher_arbre_droite_Amiga_shadow=1;
			pos_x_dest_sprite_arbre_droite_Amiga_Shadow = bord_droit_sprites_Amiga_Shadow;
		}
		
		if (randomNumber ==4  and flag_afficher_puit_Amiga_shadow==0) 
		{
			//TRACE("randomNumber puit: %d\n",randomNumber);
			flag_afficher_puit_Amiga_shadow=1;
			pos_x_dest_sprite_puit_Amiga_Shadow = bord_droit_sprites_Amiga_Shadow;
		}

		// affichage sprites Amiga

		if (flag_afficher_arbre_droite_Amiga_shadow==1)
		{
			affiche_sprite_Amiga_Shadow(pos_x_dest_sprite_arbre_droite_Amiga_Shadow , largeur_sprite_arbre_droite_Amiga_Shadow , hauteur_sprite_arbre_droite_Amiga_Shadow , pos_x_source_sprite_arbre_droite_Amiga_Shadow, pos_y_source_sprite_arbre_droite_Amiga_Shadow , pos_y_dest_sprite_arbre_droite_Amiga_Shadow  );

			pos_x_dest_sprite_arbre_droite_Amiga_Shadow = pos_x_dest_sprite_arbre_droite_Amiga_Shadow - 2;
			if ((pos_x_dest_sprite_arbre_droite_Amiga_Shadow+largeur_sprite_arbre_droite_Amiga_Shadow)< bord_gauche_sprites_Amiga_Shadow) 
			{
				pos_x_dest_sprite_arbre_droite_Amiga_Shadow = bord_droit_sprites_Amiga_Shadow;
				flag_afficher_arbre_droite_Amiga_shadow=0;
			}
		}



		if (flag_afficher_arbre_gauche_Amiga_shadow==1)
		{
			affiche_sprite_Amiga_Shadow(pos_x_dest_sprite_arbre_gauche_Amiga_Shadow , largeur_sprite_arbre_gauche_Amiga_Shadow , hauteur_sprite_arbre_gauche_Amiga_Shadow , pos_x_source_sprite_arbre_gauche_Amiga_Shadow, pos_y_source_sprite_arbre_gauche_Amiga_Shadow , pos_y_dest_sprite_arbre_gauche_Amiga_Shadow  );

			//affiche_arbre_gauche_Amiga_Shadow();
			pos_x_dest_sprite_arbre_gauche_Amiga_Shadow = pos_x_dest_sprite_arbre_gauche_Amiga_Shadow - 2;
			if ((pos_x_dest_sprite_arbre_gauche_Amiga_Shadow+largeur_sprite_arbre_gauche_Amiga_Shadow)< bord_gauche_sprites_Amiga_Shadow) 
			{
				pos_x_dest_sprite_arbre_gauche_Amiga_Shadow = bord_droit_sprites_Amiga_Shadow;
				flag_afficher_arbre_gauche_Amiga_shadow=0;
			}
		}

		if (flag_afficher_puit_Amiga_shadow==1)
		{
			affiche_sprite_Amiga_Shadow(pos_x_dest_sprite_puit_Amiga_Shadow , largeur_sprite_puit_Amiga_Shadow , hauteur_sprite_puit_Amiga_Shadow , pos_x_source_sprite_puit_Amiga_Shadow, pos_y_source_sprite_puit_Amiga_Shadow , pos_y_dest_sprite_puit_Amiga_Shadow  );

			pos_x_dest_sprite_puit_Amiga_Shadow = pos_x_dest_sprite_puit_Amiga_Shadow - 2;
			if ((pos_x_dest_sprite_puit_Amiga_Shadow+largeur_sprite_puit_Amiga_Shadow)< bord_gauche_sprites_Amiga_Shadow) 
			{
				pos_x_dest_sprite_puit_Amiga_Shadow = bord_droit_sprites_Amiga_Shadow;
				flag_afficher_puit_Amiga_shadow=0;
			}
		}
	
		
		
		affiche_sprites_Shadow();

		// TRACE("ligne overlay : %d %d \n", (pos_table_overlay_Shadow, pointeur_table_overlay_Shadow_u32[pos_table_overlay_Shadow]));
		affiche_overlay_Shadow(pointeur_table_overlay_Shadow_u32[pos_table_overlay_Shadow]);
		pos_table_overlay_Shadow++;
		if (pos_table_overlay_Shadow == 1411) pos_table_overlay_Shadow=0;


///////////////////////////////////////////////////// FIN routines Shadow



		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageeavopengl = (toto / TICKS_PER_FRAME)*100;

		
		
		// rotation du vecteur normal de la facce
		// multiplication par 0,0,-1 ( vecteur normal regards observateur)
		

		
		
		// tout en float
		cos_x = cos(pos_rotation_x);
		sin_x = sin(pos_rotation_x);
		cos_y = cos(pos_rotation_y);
		sin_y = sin(pos_rotation_y);
		cos_z = cos(pos_rotation_z);
		sin_z = sin(pos_rotation_z);
		
		valeur_A = cos_z * cos_y;
		valeur_B = sin_z * cos_y;
		valeur_C = -(sin_y);
		valeur_D = (cos_z * sin_y * sin_x) - (sin_z * cos_x);
		valeur_E = ( cos_z * cos_x ) + (sin_z * sin_y * sin_x);
		valeur_F = cos_y * sin_x;
		valeur_G = ( sin_z * sin_x ) + ( cos_z * sin_y * cos_x);
		valeur_H = (sin_z * sin_y * cos_x) - (cos_z * sin_x );
		valeur_I = cos_y*cos_x;

		//TRACE("valeur C : %f\n",valeur_C);
		
		
		//float vecteur_normal_x = vertex_list[0].normal[0];
		//float vecteur_normal_y = vertex_list[0].normal[1];
		//float vecteur_normal_z = vertex_list[0].normal[2];
		
		//float new_vecteur_normal_z = (( valeur_C * vecteur_normal_x) + (valeur_F * vecteur_normal_y ) + ( valeur_I * vecteur_normal_z));
		//float new_vecteur_normal_x = (( valeur_A * vecteur_normal_x) + ( valeur_D * vecteur_normal_y ) + (valeur_G * vecteur_normal_z));
		//float new_vecteur_normal_y = (( valeur_B * vecteur_normal_x) + (valeur_E * vecteur_normal_y ) + (valeur_H * vecteur_normal_z));
		
		
		// observateur : 0,0,-1

		face_la_plus_visible=-1;
		normal_max=0;
		face0visible = facevisible ( 0,0);
		face1visible = facevisible ( 6,1);
		face2visible = facevisible ( 12,2);
		face3visible = facevisible ( 18,3);
		face4visible = facevisible ( 24,4);
		face5visible = facevisible ( 30,5);

		#define inc_maximum_music 0.01f
		if (mode_manuel == 1 and (abs(increment_pos_rotation_x) <  inc_maximum_music) and (abs(increment_pos_rotation_y) <  inc_maximum_music) and (abs(increment_pos_rotation_z) <  inc_maximum_music)  ) 
		{
			// TRACE("increment_pos_rotation_z : %f\n",increment_pos_rotation_z);
			numero_mod = face_la_plus_visible;
		}


		// TRACE("face_la_plus_visible : %d\n",face_la_plus_visible);
		
		//face0visible = facevisible ( vertex_list[0].normal[0],vertex_list[0].normal[1],vertex_list[0].normal[2],vertex_list[0].position[0],vertex_list[0].position[1],vertex_list[0].position[2]);
		//face1visible = facevisible ( vertex_list[6].normal[0],vertex_list[6].normal[1],vertex_list[6].normal[2],vertex_list[6].position[0],vertex_list[6].position[1],vertex_list[6].position[2]);
		//face2visible = facevisible ( vertex_list[12].normal[0],vertex_list[12].normal[1],vertex_list[12].normal[2],vertex_list[12].position[0],vertex_list[12].position[1],vertex_list[12].position[2]);
		//face3visible = facevisible ( vertex_list[18].normal[0],vertex_list[18].normal[1],vertex_list[18].normal[2],vertex_list[18].position[0],vertex_list[18].position[1],vertex_list[18].position[2]);
		//face4visible = facevisible ( vertex_list[24].normal[0],vertex_list[24].normal[1],vertex_list[24].normal[2],vertex_list[24].position[0],vertex_list[24].position[1],vertex_list[24].position[2]);
		//face5visible = facevisible ( vertex_list[30].normal[0],vertex_list[30].normal[1],vertex_list[30].normal[2],vertex_list[30].position[0],vertex_list[30].position[1],vertex_list[30].position[2]);
		
		//glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 0 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
		//glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*1 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
		//glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*2 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
		//glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*3 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
		//glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*4 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
		//glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*5 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );

		int nbfaces = face0visible + face1visible + face2visible +face3visible + face4visible + face5visible;
		if (nbfaces > nbfacesmaxi) 
		{ nbfacesmaxi = nbfaces;}
		
		// charge la texture
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, s_tex);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_texture_largeur, buffer_texture_hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );

		
		// copie 1er ecran
		
		//TRACE("face 0 visible : %d\n", face0visible);
		//TRACE("face 1 visible : %d\n", face1visible);
		//TRACE("face 2 visible : %d\n", face2visible);
		//TRACE("face 3 visible : %d\n", face3visible);
		//TRACE("face 4 visible : %d\n", face4visible);
		//TRACE("face 5 visible : %d\n", face5visible);
		
		if (face0visible == 1) 
			{
			//TRACE("face 0 visible");
			glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 0 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
			}
			
			
		if (face1visible == 1) 
		{
			u32* pointeur_temp_source = (u32*) malloc(hauteur_fullscreen_TCB* largeur_fullscreen_TCB *4 );
			
			memset(pointeur_temp_source, 0, ligne_debut_remplissage_fond*largeur_fullscreen_TCB*4);
			
			///////////////////////////////////////////////////
			// recopie le buffer double dans un buffer simple avec le logo
			///////////////////////////////////////////////////
			
			int pos_destination = 0;
			int pos_source=0;
		
			// copie du logo
			// ligne_debut_logo * 2 
			pos_destination = pos_destination + (ligne_debut_logo	*largeur_fullscreen_TCB );
			for (int y=0; y<hauteur_logo_fullscreen_TCB; y++)
			{
				for (int x=0; x<largeur_fullscreen_TCB; x++)
				{
					u32 pixel = buffer_logo[pos_source];
					pos_source++;
					pointeur_temp_source[pos_destination] = pixel;
					pos_destination++;
					
				}
				// pos_destination = pos_destination + FB_WIDTH*2 - (largeur_fullscreen_TCB*2);
			}
			
			
			
			
			pos_source = ligne_debut_remplissage_fond * largeur_fullscreen_TCB*2;
			pos_destination = (ligne_debut_remplissage_fond  * largeur_fullscreen_TCB);
			// on prend en compte le parallax
			pos_source = pos_source + (position_parallax_fullscreen_TCB * hauteur_fullscreen_TCB * largeur_fullscreen_TCB * 2)+pos_scrolling_horizontal_dans_ecran;
			//TRACE("pointeur_buffer_ecrans_scrolling : %X\n",pointeur_buffer_ecrans_scrolling);
			//TRACE(" pos_sourcepointeur_buffer_ecrans_scrolling : %d\n",pos_source);
		

			for (int x=0; x<hauteur_fullscreen_TCB-( ligne_debut_remplissage_fond ); x++)
			{
				for (int y=0; y<largeur_fullscreen_TCB;y++)
				{
					u32 pixel = pointeur_buffer_ecrans_scrolling[pos_source];
					pointeur_temp_source[pos_destination] = pixel;
					pos_source++;
					pos_destination++;
				}
			// on saute le double ecran en largeur ( dans le buffer)
			pos_source = pos_source + largeur_fullscreen_TCB;
			// ligne suivante a l'ecran
			// pos_destination = pos_destination + FB_WIDTH*2 - (largeur_fullscreen_TCB*2);
			}
			
			
			
			glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720 , largeur_fullscreen_TCB , hauteur_fullscreen_TCB, GL_RGBA, GL_UNSIGNED_BYTE, pointeur_temp_source);

			free(pointeur_temp_source);
			
		}
		if (face2visible == 1) 
		{
			//TRACE("face 2 visible");
			glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*2 , largeur_sprites_TCB , hauteur_sprites_TCB, GL_RGBA, GL_UNSIGNED_BYTE,buffer_ecran );
		}
		
		
		if (face3visible == 1) glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*3 , largeur_megascroll_ULM , hauteur_megascroll_ULM, GL_RGBA, GL_UNSIGNED_BYTE,buffer_ecran_megascroll_ULM );
		
		
		
		if (face4visible == 1) glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*4 , largeur_TechTech , hauteur_TechTech, GL_RGBA, GL_UNSIGNED_BYTE,buffer_ecran_TechTech );
		
		
		if (face5visible == 1) glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*5 , largeur_Shadow , hauteur_Shadow, GL_RGBA, GL_UNSIGNED_BYTE,buffer_ecran_Shadow );
		
		//glTexSubImage2D(GL_TEXTURE_2D,0, 0 , 720*5 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
		
		// copie 2eme ecran
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0 , 720 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer+(boink_screen_largeur*boink_screen_hauteur*4) );
		//int screen=5;
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0 , screen*720 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer );
		
		
				// copie 2eme ecran
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0 , 720 , boink_screen_largeur , boink_screen_hauteur, GL_RGBA, GL_UNSIGNED_BYTE,texture_screen_buffer+(boink_screen_largeur*boink_screen_hauteur*4) );
		
		// TRACE("--------------------------------------------------------------------------");
		
		nbvbl++;

    	currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageapresloadtexture = (toto / TICKS_PER_FRAME)*100;		
	   
		
		
	// partie opengl
		
		// Update our scene
        sceneUpdate();
		

		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageapressceneupdate = (toto / TICKS_PER_FRAME)*100;

		CLSRender();
		
		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageapresCLSrender = (toto / TICKS_PER_FRAME)*100;

		
		
        // Render stuff!
        sceneRender();
		

	// partie opengl
		
		
		
		posx2D+=sensx*4;
		
		if(posx2D>310) {sensx=-1;}
		if(posx2D<40-320) {sensx=1;}
		
		
		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentageeNS = (toto / TICKS_PER_FRAME)*100;

		eglSwapBuffers(s_display, s_surface);
		
		currentTick = armGetSystemTick();
		diffrenceTick = currentTick-previousTick;
		differenceNS = armTicksToNs(diffrenceTick);
		toto = (float) differenceNS;
		pourcentagefinal = (toto / TICKS_PER_FRAME)*100;

		if (luminosite < 1.0f ) luminosite = luminosite +0.005f;
	
        // We're done rendering, so we end the frame here.
        // framebufferEnd(&fb);	
	
	}
	
	// Deinitialize our scene
    sceneExit();

	deinitEgl();
 
	// Retrieve the default window
	//NWindow* win = nwindowGetDefault();
	win = nwindowGetDefault();
 
	framebufferCreate(&fb, win, FB_WIDTH, FB_HEIGHT, PIXEL_FORMAT_RGBA_8888, 2);
	framebufferMakeLinear(&fb);
	
	for (int i=0;i<10;i++) svcSleepThread(7000000); 
	
	framebufferClose(&fb);

 
	// Initialize EGL on the default window
    if (!initEgl(nwindowGetDefault()))
        return EXIT_FAILURE;

    // Load OpenGL routines using glad
    gladLoadGL();

	
	
	// outtro
		// decompress la fonte
	//stbi_set_flip_vertically_on_load(true);
	pointeur_font_outtro = stbi_load_from_memory((const stbi_uc*) _32x32_png, _32x32_png_size, &width, &height, &nchan, 4);
	pointeur_font_outtro_u32 = (u32*) pointeur_font_outtro;

	// buffer pour l'ecran
	buffer_ecran_outtro = (u32*) malloc(largeur_texture_outtro*(hauteur_texture_outtro+32)*4);
	memset(buffer_ecran_outtro, couleur_fond, largeur_texture_outtro*(hauteur_texture_outtro+32)*4);
	
	// Initialize our scene
    sceneInit_outtro();

	numero_mod=6;

    // Main graphics loop
	float temps=0.0f;
	int termine=0;
	int pos_curseur_x=0;
	int pos_curseur_y=0;
	int pos_texte = 0;
	int	rythme=0;
	int max_rythme=9;
	int clignotecurseur=0;
	int lettre;
    while (appletMainLoop())
    {
        // Get and process input
        padUpdate(&pad);
        u32 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus)
            break;

		

		// affiche le texte
		
		if (termine==0 and rythme==0)
		{
			lettre = TEXTE_outtro[pos_texte];
		
			if (lettre =='$' )
			{
				termine=1;
			}
			else
			{
				lettre = lettre - 32;
		
				int pos_y_lettre = (int) lettre / 10;
				int pos_x_lettre = ( lettre - ( pos_y_lettre *10));
			
			
				int pos_source_lettre_outtro = (pos_x_lettre*32) + ( pos_y_lettre * 32 * 320);
			
				int pos_dest_lettre_outtro = (pos_curseur_x *32)+bord_gauche_texture_outtro+ ( ( pos_curseur_y ) * largeur_texture_outtro * 32)+(bord_haut_texture_outtro*largeur_texture_outtro);
			
				for (int y = 0; y<32;y++)
				{
					for (int x=0;x<32;x++)
					{
						u32 pixel= pointeur_font_outtro_u32[pos_source_lettre_outtro];
						if (pixel != 0xFF000000 ) buffer_ecran_outtro[pos_dest_lettre_outtro]=pixel;
						pos_dest_lettre_outtro++;
						pos_source_lettre_outtro++;
					}
					pos_dest_lettre_outtro = pos_dest_lettre_outtro + largeur_texture_outtro - 32;
					pos_source_lettre_outtro = pos_source_lettre_outtro + ( 32*10) -32;
				}
			}
			pos_texte++;
			pos_curseur_x++;
			if ( pos_curseur_x> 40)
			{
				pos_curseur_x=0;
				pos_curseur_y++;
			}		
		}
		
		if (clignotecurseur==0)
		{
			clignotecurseur=1;
			lettre = 13;
			int pos_y_lettre = (int) lettre / 10;
			int pos_x_lettre = ( lettre - ( pos_y_lettre *10));
			
			
			int pos_source_lettre_outtro = (pos_x_lettre*32) + ( pos_y_lettre * 32 * 320);
			
			int pos_dest_lettre_outtro = (pos_curseur_x *32)+bord_gauche_texture_outtro+ ( ( pos_curseur_y ) * largeur_texture_outtro * 32)+(bord_haut_texture_outtro*largeur_texture_outtro);
			
			for (int y = 0; y<32;y++)
			{
				for (int x=0;x<32;x++)
				{
					u32 pixel= pointeur_font_outtro_u32[pos_source_lettre_outtro];
					if (pixel != 0xFF000000 ) 
					{
						buffer_ecran_outtro[pos_dest_lettre_outtro]=pixel;
					}
					else
					{
						buffer_ecran_outtro[pos_dest_lettre_outtro]=0x00000000;
					}
					
					pos_dest_lettre_outtro++;
					pos_source_lettre_outtro++;
				}
				pos_dest_lettre_outtro = pos_dest_lettre_outtro + largeur_texture_outtro - 32;
				pos_source_lettre_outtro = pos_source_lettre_outtro + ( 32*10) -32;
			}
		}
		
		else
		{
			clignotecurseur=0;
			
			int pos_y_lettre = (int) lettre / 10;
				int pos_x_lettre = ( lettre - ( pos_y_lettre *10));
			
			
				int pos_source_lettre_outtro = (pos_x_lettre*32) + ( pos_y_lettre * 32 * 320);
			
				int pos_dest_lettre_outtro = (pos_curseur_x *32)+bord_gauche_texture_outtro+ ( ( pos_curseur_y ) * largeur_texture_outtro * 32)+(bord_haut_texture_outtro*largeur_texture_outtro);
			
				for (int y = 0; y<32;y++)
				{
					for (int x=0;x<32;x++)
					{
						u32 pixel= pointeur_font_outtro_u32[pos_source_lettre_outtro];
						buffer_ecran_outtro[pos_dest_lettre_outtro]=couleur_fond;
						pos_dest_lettre_outtro++;
						pos_source_lettre_outtro++;
					}
					pos_dest_lettre_outtro = pos_dest_lettre_outtro + largeur_texture_outtro - 32;
					pos_source_lettre_outtro = pos_source_lettre_outtro + ( 32*10) -32;
				}
		}



		
		
		
		rythme=rythme-1;
		if (rythme==-1) rythme = max_rythme;

		glUniform1i(loc_textureouttro,  1);
		glActiveTexture(GL_TEXTURE0 + 1);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, largeur_texture_outtro, hauteur_texture_outtro, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_ecran_outtro);
		glTexSubImage2D(GL_TEXTURE_2D , 0, 0 , 0 , largeur_texture_outtro , hauteur_texture_outtro, GL_RGBA, GL_UNSIGNED_BYTE, buffer_ecran_outtro );
	

		// update time
		glUniform1f(loc_itime, temps);
		

		
        sceneRender_outtro();
		
		nbvbl++;

		temps=temps+ float(1.0f/60.0f);
        eglSwapBuffers(s_display, s_surface);
		
		
    }

	
	
	
	///// -------------------------------- ///// -------------------------------- ///// -------------------------------- ///// -------------------------------- 
	finaltick = armGetSystemTick();
	
	u64 dureetick = finaltick - initialTick;
	differenceNS = armTicksToNs(dureetick);
	float differenceSec = differenceNS / 1000000000.0;
	float FPSfinal = nbvbl / differenceSec;
	TRACE("================================>FPS :  %f\n", FPSfinal);
	TRACE("\n");
	// sortie
	
	running = false;
    threadWaitForExit(&id_thread_playmodule);
    threadClose(&id_thread_playmodule);
	
	 // Stop audio playback.
    rc = audoutStopAudioOut();
    //printf("audoutStopAudioOut() returned 0x%x\n", rc);

    // Terminate the default audio output device.
    audoutExit();
	
    // Free resources
	hxcmod_unload( modctx_enigma );
	free(modctx_enigma);
	hxcmod_unload( modctx_ronken );
	free(modctx_ronken);
	hxcmod_unload( modctx_knulla );
	free(modctx_knulla);
	ymMusicDestroy(pMusic_beast);
	ymMusicDestroy(pMusic_spritesTCB);
	ymMusicDestroy(pMusic_ULM);

	
	#ifdef feedback_visibilite
	for(int i=0;i<6;i++)
	{
		int ii=i*6;
		TRACE("feedback %i / 1: %f 2: %f 3:%f 4: %f 5: %f 6: %f \n", i,feedback[0+ii], feedback[1+ii], feedback[2+ii], feedback[3+ii], feedback[4+ii], feedback[5+ii]);
	}
	#endif

/*
	TRACE("normal vertex 1 Z : %f\n",vertex_list[15].normal[2]);
	// vec3 inPos;
	glm::vec3 inPos_vec3 = glm::make_vec3(vertex_list[15].position);
	// vec3 inNormal
	glm::vec3 inNormal = glm::make_vec3(vertex_list[15].normal);
	TRACE("normal vertex 0 Z en vec3: %f\n",inNormal[2]);
	glm::mat3 mdlvMtx_mat3;
	mdlvMtx_mat3 = glm::mat3(mdlvMtx);
	// vec3 normal = normalize(mat3(mdlvMtx) * inNormal);
	glm::vec3 normal = mdlvMtx_mat3 * inNormal;
	normal = glm::normalize(normal);
	//float z = (1.0 + normal.z) / 2.0;
	float zshader = normal.z;
	zshader = (1.0 + zshader) / 2.0;
	// vec4 pos = mdlvMtx * vec4(inPos, 1.0);
	glm::vec4 pos_vec4;
	pos_vec4 = mdlvMtx * glm::vec4(inPos_vec3, 1.0);
	// vtxView = -pos.xyz;
	glm::vec3 vtxView;
	vtxView = -(pos_vec4.xyz());
	glm::vec3 viewVec = glm::normalize(vtxView);
	// vtxNormalQuat = vec4(1.0, 0.0, 0.0, 0.0);
	glm::vec4 vtxNormalQuat = glm::vec4(1.0, 0.0, 0.0, 0.0);
	if (zshader > 0.0)
        {
			// vtxNormalQuat.z = sqrt(z);
            vtxNormalQuat.z = sqrt(zshader);
			// vtxNormalQuat.xy = normal.xy / (2.0 * vtxNormalQuat.z);
            vtxNormalQuat.x = normal.x / (2.0 * vtxNormalQuat.z);
			vtxNormalQuat.y = normal.y / (2.0 * vtxNormalQuat.z);
        }
	// vec4 normquat = normalize(vtxNormalQuat);
	glm::vec4 normquat = glm::normalize(vtxNormalQuat);
	// vec3 normal2 = quatrotate(normquat, vec3(0.0, 0.0, 1.0));
	// vec3 quatrotate(vec4 q, vec3 v)
	// {
	// return v + 2.0*cross(q.xyz, cross(q.xyz, v) + q.w*v);
	// toto1 = q.xyz
	// toto2 = cross(toto1, v)
	// toto3 = toto2 + q.w*v
	// toto4 = q.xyz
	// v + 2.0*cross(toto4, toto3);
	glm::vec3 normal2;
	glm::vec3 quatrotate_v = glm::vec3(0.0, 0.0, 1.0);
	glm::vec3 toto1 = normquat.xyz();
	glm::vec3 toto2 = glm::cross(toto1, quatrotate_v);
	glm::vec3 toto3 = normquat.w * quatrotate_v;
	toto3 = toto2 + toto3;
	glm::vec3 toto4 = normquat.xyz();
	glm::vec3 resultat_quatrotate;
	resultat_quatrotate = glm::cross(toto4, toto3);
	resultat_quatrotate =  resultat_quatrotate * 2.0f;
	resultat_quatrotate =  quatrotate_v + resultat_quatrotate;
	float edz = glm::dot(resultat_quatrotate, viewVec);
	edz = glm::max(edz, 0.0f);
	TRACE("EDZ calculé pourri 15 2,4: %f\n",edz);
	//normal2 = quatrotate_v + 2.0*cross(normquat.xyz(), cross(normquat.xyz(), quatrotate_v) + normquat.w*quatrotate_v);
*/
	#ifdef feedback_visibilite
		TRACE("face 0 visible : %d\n", face0visible);
		TRACE("face 1 visible : %d\n", face1visible);
		TRACE("face 2 visible : %d\n", face2visible);
		TRACE("face 3 visible : %d\n", face3visible);
		TRACE("face 4 visible : %d\n", face4visible);
		TRACE("face 5 visible : %d\n", face5visible);
	#endif
	
	/*
	TRACE("pourcentage avant CLS : %2f %%\n",pourcentageavantCLS);
	TRACE("pourcentage apres CLS : %2f %%\n",pourcentageCLS);
	TRACE("pourcentage Calcul : %2f %%\n",pourcentageCalc);
	TRACE("pourcentage avant opengl : %2f %%\n",pourcentageeavopengl);
	TRACE("pourcentage apres load texture : %2f %%\n",pourcentageapresloadtexture);
	TRACE("pourcentage apres scene update : %2f %%\n",pourcentageapressceneupdate);
	TRACE("pourcentage apres CLS Buffer Render : %2f %%\n",pourcentageapresCLSrender);
	TRACE("pourcentage avant draw  : %2f %%\n",pourcentageavantdraw);
	TRACE("pourcentage apres apres draw : %2f %%\n",pourcentageapresdraw);
	
	TRACE("pourcentage total avant synchro openGL: %2f %%\n",pourcentageeNS);
	TRACE("pourcentage apres opengl terminé : %2f %%\n",pourcentagefinal);
	*/
	
	#ifdef feedback_visibilite
		TRACE("nombre de face maximum :  %d\n",nbfacesmaxi);
	#endif
	
	//GLint pformat;
	//GLenum internalformat;
	//internalformat = GL_RGBA8;
	//glGetInternalformativ(GL_TEXTURE_2D, internalformat, GL_TEXTURE_IMAGE_FORMAT, 1, &pformat);
	//TRACE("informativ : %d\n",pformat);
	//TRACE("GL_RGBA8 : %d\n",GL_RGBA8);
	//TRACE("GL_RGBA : %d\n",GL_RGBA);
	
	//TRACE("vertex_list_count :  %u\n",vertex_list_count);
	//TRACE("sizeof(vertex_list[0])  %u\n",(int) sizeof(vertex_list[0]));
	 // vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))

	//TRACE("%s\n",glGetString(GL_VERSION));
	

	sceneExit_outtro();

    // Deinitialize EGL
    deinitEgl();
	
	free(back_rgba);
	free(pointeur_buffer_ecrans_scrolling);

	
	//framebufferClose(&fb);

	return EXIT_SUCCESS;
    
}