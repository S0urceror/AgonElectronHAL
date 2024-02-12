#include "ppi-8255.h"
#include "hal.h"
#include <fabutils.h>


vk_to_msx_matrix msx_keys_to_matrix[] = { 
    { 00, 0b00000000 }, // VK_NONE,            /**< No character (marks the first virtual key) */
    { 8, 0b00000001 }, // VK_SPACE,           /**< Space */

    { 0, 0b00000001 }, // VK_0,               /**< Number 0 */
    { 0, 0b00000010 }, // VK_1,               /**< Number 1 */
    { 0, 0b00000100 }, // VK_2,               /**< Number 2 */
    { 0, 0b00001000 }, // VK_3,               /**< Number 3 */
    { 0, 0b00010000 }, // VK_4,               /**< Number 4 */
    { 0, 0b00100000 }, // VK_5,               /**< Number 5 */
    { 0, 0b01000000 }, // VK_6,               /**< Number 6 */
    { 0, 0b10000000 }, // VK_7,               /**< Number 7 */
    { 1, 0b00000001 }, // VK_8,               /**< Number 8 */
    { 1, 0b00000010 }, // VK_9,               /**< Number 9 */
    { 9, 0b00001000 }, // VK_KP_0,            /**< Keypad number 0 */
    { 9, 0b00010000 }, // VK_KP_1,            /**< Keypad number 1 */
    { 9, 0b00100000 }, // VK_KP_2,            /**< Keypad number 2 */
    { 9, 0b01000000 }, // VK_KP_3,            /**< Keypad number 3 */
    { 9, 0b10000000 }, // VK_KP_4,            /**< Keypad number 4 */
    { 10, 0b00000001 }, // VK_KP_5,            /**< Keypad number 5 */
    { 10, 0b00000010 }, // VK_KP_6,            /**< Keypad number 6 */
    { 10, 0b00000100 }, // VK_KP_7,            /**< Keypad number 7 */
    { 10, 0b00001000 }, // VK_KP_8,            /**< Keypad number 8 */
    { 10, 0b00010000 }, // VK_KP_9,            /**< Keypad number 9 */

    { 2, 0b01000000 }, // VK_a,               /**< Lower case letter 'a' */
    { 2, 0b10000000 }, // VK_b,               /**< Lower case letter 'b' */
    { 3, 0b00000001 }, // VK_c,               /**< Lower case letter 'c' */
    { 3, 0b00000010 }, // VK_d,               /**< Lower case letter 'd' */
    { 3, 0b00000100 }, // VK_e,               /**< Lower case letter 'e' */
    { 3, 0b00001000 }, // VK_f,               /**< Lower case letter 'f' */
    { 3, 0b00010000 }, // VK_g,               /**< Lower case letter 'g' */
    { 3, 0b00100000 }, // VK_h,               /**< Lower case letter 'h' */
    { 3, 0b01000000 }, // VK_i,               /**< Lower case letter 'i' */
    { 3, 0b10000000 }, // VK_j,               /**< Lower case letter 'j' */
    { 4, 0b00000001 }, // VK_k,               /**< Lower case letter 'k' */
    { 4, 0b00000010 }, // VK_l,               /**< Lower case letter 'l' */
    { 4, 0b00000100 }, // VK_m,               /**< Lower case letter 'm' */
    { 4, 0b00001000 }, // VK_n,               /**< Lower case letter 'n' */
    { 4, 0b00010000 }, // VK_o,               /**< Lower case letter 'o' */
    { 4, 0b00100000 }, // VK_p,               /**< Lower case letter 'p' */
    { 4, 0b01000000 }, // VK_q,               /**< Lower case letter 'q' */
    { 4, 0b10000000 }, // VK_r,               /**< Lower case letter 'r' */
    { 5, 0b00000001 }, // VK_s,               /**< Lower case letter 's' */
    { 5, 0b00000010 }, // VK_t,               /**< Lower case letter 't' */
    { 5, 0b00000100 }, // VK_u,               /**< Lower case letter 'u' */
    { 5, 0b00001000 }, // VK_v,               /**< Lower case letter 'v' */
    { 5, 0b00010000 }, // VK_w,               /**< Lower case letter 'w' */
    { 5, 0b00100000 }, // VK_x,               /**< Lower case letter 'x' */
    { 5, 0b01000000 }, // VK_y,               /**< Lower case letter 'y' */
    { 5, 0b10000000 }, // VK_z,               /**< Lower case letter 'z' */
    { 2, 0b01000000 }, // VK_A,               /**< Upper case letter 'A' */
    { 2, 0b10000000 }, // VK_B,               /**< Upper case letter 'B' */
    { 3, 0b00000001 }, // VK_C,               /**< Upper case letter 'C' */
    { 3, 0b00000010 }, // VK_D,               /**< Upper case letter 'D' */
    { 3, 0b00000100 }, // VK_E,               /**< Upper case letter 'E' */
    { 3, 0b00001000 }, // VK_F,               /**< Upper case letter 'F' */
    { 3, 0b00010000 }, // VK_G,               /**< Upper case letter 'G' */
    { 3, 0b00100000 }, // VK_H,               /**< Upper case letter 'H' */
    { 3, 0b01000000 }, // VK_I,               /**< Upper case letter 'I' */
    { 3, 0b10000000 }, // VK_J,               /**< Upper case letter 'J' */
    { 4, 0b00000001 }, // VK_K,               /**< Upper case letter 'K' */
    { 4, 0b00000010 }, // VK_L,               /**< Upper case letter 'L' */
    { 4, 0b00000100 }, // VK_M,               /**< Upper case letter 'M' */
    { 4, 0b00001000 }, // VK_N,               /**< Upper case letter 'N' */
    { 4, 0b00010000 }, // VK_O,               /**< Upper case letter 'O' */
    { 4, 0b00100000 }, // VK_P,               /**< Upper case letter 'P' */
    { 4, 0b01000000 }, // VK_Q,               /**< Upper case letter 'Q' */
    { 4, 0b10000000 }, // VK_R,               /**< Upper case letter 'R' */
    { 5, 0b00000001 }, // VK_S,               /**< Upper case letter 'S' */
    { 5, 0b00000010 }, // VK_T,               /**< Upper case letter 'T' */
    { 5, 0b00000100 }, // VK_U,               /**< Upper case letter 'U' */
    { 5, 0b00001000 }, // VK_V,               /**< Upper case letter 'V' */
    { 5, 0b00010000 }, // VK_W,               /**< Upper case letter 'W' */
    { 5, 0b00100000 }, // VK_X,               /**< Upper case letter 'X' */
    { 5, 0b01000000 }, // VK_Y,               /**< Upper case letter 'Y' */
    { 5, 0b10000000 }, // VK_Z,               /**< Upper case letter 'Z' */

    { 2, 0b00000010 }, // VK_GRAVEACCENT,     /**< Grave accent: ` */
    { 0, 0b00000000 }, // VK_ACUTEACCENT,     /**< Acute accent: ´ */
    { 2, 0b00000001 }, // VK_QUOTE,           /**< Quote: ' */
    { 2, 0b00000001 }, // VK_QUOTEDBL,        /**< Double quote: " */
    { 1, 0b00001000 }, // VK_EQUALS,          /**< Equals: = */
    { 1, 0b00000100 }, // VK_MINUS,           /**< Minus: - */
    { 10, 0b00100000 }, // VK_KP_MINUS,        /**< Keypad minus: - */
    { 1, 0b00001000 }, // VK_PLUS,            /**< Plus: + */
    { 9, 0b00000010 }, // VK_KP_PLUS,         /**< Keypad plus: + */
    { 9, 0b00000001 }, // VK_KP_MULTIPLY,     /**< Keypad multiply: * */
    { 1, 0b00000001 }, // VK_ASTERISK,        /**< Asterisk: * */
    { 1, 0b00010000 }, // VK_BACKSLASH,       /**< Backslash: \ */
    { 9, 0b00000100 }, // VK_KP_DIVIDE,       /**< Keypad divide: / */
    { 2, 0b00010000 }, // VK_SLASH,           /**< Slash: / */
    { 10, 0b10000000 }, // VK_KP_PERIOD,       /**< Keypad period: . */
    { 2, 0b00001000 }, // VK_PERIOD,          /**< Period: . */
    { 1, 0b10000000 }, // VK_COLON,           /**< Colon: : */
    { 2, 0b00000100 }, // VK_COMMA,           /**< Comma: , */
    { 1, 0b10000000 }, // VK_SEMICOLON,       /**< Semicolon: ; */
    { 0, 0b10000000 }, // VK_AMPERSAND,       /**< Ampersand: & */
    { 1, 0b00010000 }, // VK_VERTICALBAR,     /**< Vertical bar: | */
    { 0, 0b00001000 }, // VK_HASH,            /**< Hash: # */
    { 0, 0b00000100 }, // VK_AT,              /**< At: @ */
    { 0, 0b01000000 }, // VK_CARET,           /**< Caret: ^ */
    { 0, 0b00010000 }, // VK_DOLLAR,          /**< Dollar: $ */
    { 0, 0b00010000 }, // VK_POUND,           /**< Pound: £ */
    { 0, 0b00010000 }, // VK_EURO,            /**< Euro: € */
    { 0, 0b00100000 }, // VK_PERCENT,         /**< Percent: % */
    { 0, 0b00000010 }, // VK_EXCLAIM,         /**< Exclamation mark: ! */
    { 2, 0b00010000 }, // VK_QUESTION,        /**< Question mark: ? */
    { 1, 0b00100000 }, // VK_LEFTBRACE,       /**< Left brace: { */
    { 1, 0b01000000 }, // VK_RIGHTBRACE,      /**< Right brace: } */
    { 1, 0b00100000 }, // VK_LEFTBRACKET,     /**< Left bracket: [ */
    { 1, 0b01000000 }, // VK_RIGHTBRACKET,    /**< Right bracket: ] */
    { 1, 0b00000010 }, // VK_LEFTPAREN,       /**< Left parenthesis: ( */
    { 0, 0b00000001 }, // VK_RIGHTPAREN,      /**< Right parenthesis: ) */
    { 2, 0b00000100 }, // VK_LESS,            /**< Less: < */
    { 2, 0b00001000 }, // VK_GREATER,         /**< Greater: > */
    { 1, 0b00000100 }, // VK_UNDERSCORE,      /**< Underscore: _ */
    { 2, 0b00010000 }, // VK_DEGREE,          /**< Degree: ° */
    { 0, 0b00001000 }, // VK_SECTION,         /**< Section: § */
    { 2, 0b00000010 }, // VK_TILDE,           /**< Tilde: ~ */
    { 0, 0b00000000 }, // VK_NEGATION,        /**< Negation: ¬ */
    { 6, 0b00000001 }, // VK_LSHIFT,          /**< Left SHIFT */
    { 6, 0b00000001 }, // VK_RSHIFT,          /**< Right SHIFT */
    { 6, 0b00000100 }, // VK_LALT,            /**< Left ALT */
    { 6, 0b00000100 }, // VK_RALT,            /**< Right ALT */
    { 6, 0b00000010 }, // VK_LCTRL,           /**< Left CTRL */
    { 6, 0b00000010 }, // VK_RCTRL,           /**< Right CTRL */
    { 6, 0b00010000 }, // VK_LGUI,            /**< Left GUI */
    { 6, 0b00010000 }, // VK_RGUI,            /**< Right GUI */

    { 7, 0b00000100 }, // VK_ESCAPE,          /**< ESC */

    { 0, 0b00000000 }, // VK_PRINTSCREEN,     /**< PRINTSCREEN */
    { 0, 0b00000000 }, // VK_SYSREQ,          /**< SYSREQ */

    { 8, 0b00000100 }, // VK_INSERT,          /**< INS */
    { 8, 0b00000100 }, // VK_KP_INSERT,       /**< Keypad INS */
    { 8, 0b00001000 }, // VK_DELETE,          /**< DEL */
    { 8, 0b00001000 }, // VK_KP_DELETE,       /**< Keypad DEL */
    { 7, 0b00100000 }, // VK_BACKSPACE,       /**< Backspace */
    { 8, 0b00000010 }, // VK_HOME,            /**< HOME */
    { 8, 0b00000010 }, // VK_KP_HOME,         /**< Keypad HOME */
    { 0, 0b00000000 }, // VK_END,             /**< END */
    { 0, 0b00000000 }, // VK_KP_END,          /**< Keypad END */
    { 7, 0b00010000 }, // VK_PAUSE,           /**< PAUSE */
    { 7, 0b00010000 }, // VK_BREAK,           /**< CTRL + PAUSE */
    { 0, 0b00000000 }, // VK_SCROLLLOCK,      /**< SCROLLLOCK */
    { 0, 0b00000000 }, // VK_NUMLOCK,         /**< NUMLOCK */
    { 6, 0b00001000 }, // VK_CAPSLOCK,        /**< CAPSLOCK */
    { 7, 0b00001000 }, // VK_TAB,             /**< TAB */
    { 7, 0b10000000 }, // VK_RETURN,          /**< RETURN */
    { 7, 0b10000000 }, // VK_KP_ENTER,        /**< Keypad ENTER */
    { 6, 0b00000100 }, // VK_APPLICATION,     /**< APPLICATION / MENU key */
    { 0, 0b00000000 }, // VK_PAGEUP,          /**< PAGEUP */
    { 0, 0b00000000 }, // VK_KP_PAGEUP,       /**< Keypad PAGEUP */
    { 0, 0b00000000 }, // VK_PAGEDOWN,        /**< PAGEDOWN */
    { 0, 0b00000000 }, // VK_KP_PAGEDOWN,     /**< Keypad PAGEDOWN */
    { 8, 0b00100000 }, // VK_UP,              /**< Cursor UP */
    { 8, 0b00100000 }, // VK_KP_UP,           /**< Keypad cursor UP  */
    { 8, 0b01000000 }, // VK_DOWN,            /**< Cursor DOWN */
    { 8, 0b01000000 }, // VK_KP_DOWN,         /**< Keypad cursor DOWN */
    { 8, 0b00010000 }, // VK_LEFT,            /**< Cursor LEFT */
    { 8, 0b00010000 }, // VK_KP_LEFT,         /**< Keypad cursor LEFT */
    { 8, 0b10000000 }, // VK_RIGHT,           /**< Cursor RIGHT */
    { 8, 0b10000000 }, // VK_KP_RIGHT,        /**< Keypad cursor RIGHT */
    { 0, 0b00000000 }, // VK_KP_CENTER,       /**< Keypad CENTER key */

    { 6, 0b00100000 }, // VK_F1,              /**< F1 function key */
    { 6, 0b01000000 }, // VK_F2,              /**< F2 function key */
    { 6, 0b10000000 }, // VK_F3,              /**< F3 function key */
    { 7, 0b00000001 }, // VK_F4,              /**< F4 function key */
    { 7, 0b00000010 }, // VK_F5,              /**< F5 function key */
    { 0, 0b00000000 }, // VK_F6,              /**< F6 function key */
    { 0, 0b00000000 }, // VK_F7,              /**< F7 function key */
    { 0, 0b00000000 }, // VK_F8,              /**< F8 function key */
    { 0, 0b00000000 }, // VK_F9,              /**< F9 function key */
    { 0, 0b00000000 }, // VK_F10,             /**< F10 function key */
    { 0, 0b00000000 }, // VK_F11,             /**< F11 function key */
    { 0, 0b00000000 }, // VK_F12,             /**< F12 function key */

    { 0, 0b00000000 }, // VK_GRAVE_a,         /**< Grave a: à */
    { 0, 0b00000000 }, // VK_GRAVE_e,         /**< Grave e: è */
    { 0, 0b00000000 }, // VK_GRAVE_i,         /**< Grave i: ì */
    { 0, 0b00000000 }, // VK_GRAVE_o,         /**< Grave o: ò */
    { 0, 0b00000000 }, // VK_GRAVE_u,         /**< Grave u: ù */
    { 0, 0b00000000 }, // VK_GRAVE_y,         /**< Grave y: ỳ */

    { 0, 0b00000000 }, // VK_ACUTE_a,         /**< Acute a: á */
    { 0, 0b00000000 }, // VK_ACUTE_e,         /**< Acute e: é */
    { 0, 0b00000000 }, // VK_ACUTE_i,         /**< Acute i: í */
    { 0, 0b00000000 }, // VK_ACUTE_o,         /**< Acute o: ó */
    { 0, 0b00000000 }, // VK_ACUTE_u,         /**< Acute u: ú */
    { 0, 0b00000000 }, // VK_ACUTE_y,         /**< Acute y: ý */

    { 0, 0b00000000 }, // VK_GRAVE_A,		      /**< Grave A: À */
    { 0, 0b00000000 }, // VK_GRAVE_E,		      /**< Grave E: È */
    { 0, 0b00000000 }, // VK_GRAVE_I,		      /**< Grave I: Ì */
    { 0, 0b00000000 }, // VK_GRAVE_O,		      /**< Grave O: Ò */
    { 0, 0b00000000 }, // VK_GRAVE_U,		      /**< Grave U: Ù */
    { 0, 0b00000000 }, // VK_GRAVE_Y,         /**< Grave Y: Ỳ */

    { 0, 0b00000000 }, // VK_ACUTE_A,		      /**< Acute A: Á */
    { 0, 0b00000000 }, // VK_ACUTE_E,		      /**< Acute E: É */
    { 0, 0b00000000 }, // VK_ACUTE_I,		      /**< Acute I: Í */
    { 0, 0b00000000 }, // VK_ACUTE_O,		      /**< Acute O: Ó */
    { 0, 0b00000000 }, // VK_ACUTE_U,		      /**< Acute U: Ú */
    { 0, 0b00000000 }, // VK_ACUTE_Y,         /**< Acute Y: Ý */

    { 0, 0b00000000 }, // VK_UMLAUT_a,        /**< Diaeresis a: ä */
    { 0, 0b00000000 }, // VK_UMLAUT_e,        /**< Diaeresis e: ë */
    { 0, 0b00000000 }, // VK_UMLAUT_i,        /**< Diaeresis i: ï */
    { 0, 0b00000000 }, // VK_UMLAUT_o,        /**< Diaeresis o: ö */
    { 0, 0b00000000 }, // VK_UMLAUT_u,        /**< Diaeresis u: ü */
    { 0, 0b00000000 }, // VK_UMLAUT_y,        /**< Diaeresis y: ÿ */

    { 0, 0b00000000 }, // VK_UMLAUT_A,        /**< Diaeresis A: Ä */
    { 0, 0b00000000 }, // VK_UMLAUT_E,        /**< Diaeresis E: Ë */
    { 0, 0b00000000 }, // VK_UMLAUT_I,        /**< Diaeresis I: Ï */
    { 0, 0b00000000 }, // VK_UMLAUT_O,        /**< Diaeresis O: Ö */
    { 0, 0b00000000 }, // VK_UMLAUT_U,        /**< Diaeresis U: Ü */
    { 0, 0b00000000 }, // VK_UMLAUT_Y,        /**< Diaeresis Y: Ÿ */

    { 0, 0b00000000 }, // VK_CARET_a,		      /**< Caret a: â */
    { 0, 0b00000000 }, // VK_CARET_e,		      /**< Caret e: ê */
    { 0, 0b00000000 }, // VK_CARET_i,		      /**< Caret i: î */
    { 0, 0b00000000 }, // VK_CARET_o,		      /**< Caret o: ô */
    { 0, 0b00000000 }, // VK_CARET_u,		      /**< Caret u: û */
    { 0, 0b00000000 }, // VK_CARET_y,         /**< Caret y: ŷ */

    { 0, 0b00000000 }, // VK_CARET_A,		      /**< Caret A: Â */
    { 0, 0b00000000 }, // VK_CARET_E,		      /**< Caret E: Ê */
    { 0, 0b00000000 }, // VK_CARET_I,		      /**< Caret I: Î */
    { 0, 0b00000000 }, // VK_CARET_O,		      /**< Caret O: Ô */
    { 0, 0b00000000 }, // VK_CARET_U,		      /**< Caret U: Û */
    { 0, 0b00000000 }, // VK_CARET_Y,         /**< Caret Y: Ŷ */

    { 0, 0b00000000 }, // VK_CEDILLA_c,       /**< Cedilla c: ç */
    { 0, 0b00000000 }, // VK_CEDILLA_C,       /**< Cedilla C: Ç */

    { 0, 0b00000000 }, // VK_TILDE_a,         /**< Lower case tilde a: ã */
    { 0, 0b00000000 }, // VK_TILDE_o,         /**< Lower case tilde o: õ */
    { 0, 0b00000000 }, // VK_TILDE_n,		      /**< Lower case tilde n: ñ */

    { 0, 0b00000000 }, // VK_TILDE_A,         /**< Upper case tilde A: Ã */
    { 0, 0b00000000 }, // VK_TILDE_O,         /**< Upper case tilde O: Õ */
    { 0, 0b00000000 }, // VK_TILDE_N,	      /**< Upper case tilde N: Ñ */

    { 0, 0b00000000 }, // VK_UPPER_a,		      /**< primera: a */
    { 0, 0b00000000 }, // VK_ESZETT,          /**< Eszett: ß */
    { 0, 0b00000000 }, // VK_EXCLAIM_INV,     /**< Inverted exclamation mark: ! */
    { 0, 0b00000000 }, // VK_QUESTION_INV,    /**< Inverted question mark : ? */
    { 0, 0b00000000 }, // VK_INTERPUNCT,	    /**< Interpunct : · */
    { 0, 0b00000000 }, // VK_DIAERESIS,	  	  /**< Diaeresis  : ¨ */
    { 0, 0b00000000 }, // VK_SQUARE,          /**< Square     : ² */
    { 0, 0b00000000 }, // VK_CURRENCY,        /**< Currency   : ¤ */
    { 0, 0b00000000 }, // VK_MU,              /**< Mu         : µ */
    { 0, 0b00000000 }, // VK_HALF,            /**< 1/2 fraction : 1/2 */
    { 0, 0b00000000 }, // VK_MASCULIN_ORD,    /**< Masculin ordinal superscript o */ 
    { 0, 0b00000000 }, // VK_FEMININ_ORD,     /**< Feminin ordinal superscript a */  
    { 0, 0b00000000 }, // VK_LEFTGUILLEMET,   /**< Left guillemet: « */
    { 0, 0b00000000 }, // VK_RIGHTGUILLEMET,  /**< Right guillemet: » */
    { 0, 0b00000000 }, // VK_aelig,           /** Lower case aelig  : æ */
    { 0, 0b00000000 }, // VK_oslash,          /** Lower case oslash : ø */
    { 0, 0b00000000 }, // VK_aring,           /** Lower case aring  : å */

    { 0, 0b00000000 }, // VK_AELIG,           /** Upper case aelig  : Æ */
    { 0, 0b00000000 }, // VK_OSLASH,          /** Upper case oslash : Ø */
    { 0, 0b00000000 }, // VK_ARING,           /** Upper case aring  : Å */

    { 0, 0b00000000 }, // VK_DEAD_GRAVEACCENT,/** Grave accent when we need seperate code for dead key*/
    { 0, 0b00000000 }, // VK_DEAD_CARET,      /** Caret when we need seperate code for dead key*/
    { 0, 0b00000000 }, // VK_DEAD_TILDE,      /** Tilde when we need seperate code for dead key*/

// // Missing code page 1252 virtual keys
    { 0, 0b00000000 }, // VK_UPPER_1,         /** Superscript 1   : ¹ */
    { 0, 0b00000000 }, // VK_CUBE,            /** Superscript 3   : ³ */
    { 0, 0b00000000 }, // VK_CENT,            /** Cent (currency) : ¢ */

// // Japanese layout support
    { 0, 0b00000000 }, // VK_YEN,
    { 0, 0b00000000 }, // VK_MUHENKAN,
    { 0, 0b00000000 }, // VK_HENKAN,
    { 0, 0b00000000 }, // VK_KATAKANA_HIRAGANA_ROMAJI,
    { 0, 0b00000000 }, // VK_HANKAKU_ZENKAKU_KANJI,
    { 0, 0b00000000 }, // VK_SHIFT_0,

    { 0, 0b00000000 }, // VK_ASCII,           /**< Specifies an ASCII code - used when virtual key is embedded in VirtualKeyItem structure and VirtualKeyItem.ASCII is valid */
    { 0, 0b00000000 }, // VK_LAST,            // marks the last virtual key    
};

MSX_PPI8255::MSX_PPI8255 () : PPI8255()
{
    for (int i=0;i<(sizeof (rows)/sizeof (uint8_t));i++)
        rows [i]=0xff;
}
uint8_t MSX_PPI8255::read (uint8_t port)
{
    switch (port)
    {
        case 0: 
            // slot selection register, returns 0 for page 0 to 3
            return 0x00; 
        case 1: 
            // return bits belonging to row indicated by portC
            return rows[portC & 0x0f];
        case 2: 
            return portC;
        case 3: 
            return control;
    }
    return 0xff;
}

uint8_t MSX_PPI8255::record_keypress (uint8_t ascii,uint8_t modifier,uint8_t vk,uint8_t down)
{
    uint8_t mask;
    if (vk<sizeof(msx_keys_to_matrix))
    {
        current_row = msx_keys_to_matrix[vk].msx_matrix_row;
        mask = msx_keys_to_matrix[vk].msx_matrix_mask;
        if (down)
        {
            // zero the right bits defined in the mask
            mask = ~mask;
            rows[current_row] &= mask;
        }
        else
        {
            // activate the right bits defined in the mask
            rows[current_row] |= mask;
        }
    }
    return current_row;
}
uint8_t MSX_PPI8255::get_row_bits (uint8_t row)
{
    if (row<sizeof(rows))
        return rows[row];

    // error, fallback
    return 0xff; // all bits high, no keypress
}
uint8_t MSX_PPI8255::get_next_row ()
{
    uint8_t row = current_row;
    current_row++;
    if (current_row>=sizeof(rows))
        current_row = 0;
    return row;
}
SG1000_PPI8255::SG1000_PPI8255 () : PPI8255()
{
    for (int i=0;i<(sizeof (rows)/sizeof (uint16_t));i++)
        rows [i]=0xffff;
}

uint8_t SG1000_PPI8255::read (uint8_t port)
{
    // hal_printf ("PPI read port %c\r\n",port+'A'); 
    switch (port)
    {
        case 0: 
            // return low 8 bits belonging to row indicated by portC
            return uint8_t (rows[portC] & 0x00ff);
        case 1: 
            // return high 8 bits belonging to row indicated by portC
            return uint8_t ((rows[portC] & 0xff00) >> 8);
        case 2: 
            return 0xff; // return portC if you want to emulate keyboard scanning, return 0xff if you just want keypads
        case 3: 
            return 0xff;
    }
    return 0xff;
}

// SG-1000
// PPI	Port	A							Port	B		
// Rows	D0	D1	D2	D3	D4	D5	D6	D7	D0	D1	D2	D3
// 0	'1'	'Q'	'A'	'Z'	ED	','	'K'	'I'	'8'	—	—	—
// 1	'2'	'W'	'S'	'X'	SPC	'.'	'L'	'O'	'9'	—	—	—
// 2	'3'	'E'	'D'	'C'	HC	'/'	';'	'P'	'0'	—	—	—
// 3	'4'	'R'	'F'	'V'	ID	PI	':'	'@'	'-'	—	—	—
// 4	'5'	'T'	'G'	'B'	—	DA	']'	'['	'^'	—	—	—
// 5	'6'	'Y'	'H'	'N'	—	LA	CR	—	YEN	—	—	FNC
// 6	'7'	'U'	'J'	'M'	—	RA	UA	—	BRK	GRP	CTL	SHF
// 7	1U	1D	1L	1R	1TL	1TR	2U	2D	2L	2R	2TL	2TR
uint8_t SG1000_PPI8255::record_keypress (uint8_t ascii,uint8_t modifier,uint8_t vk,uint8_t down)
{
    current_row = 7;
    // only supporting row 7
    if (down!=1)
    {
        // up, make bits 1
        switch (vk)
        {
            case fabgl::VK_UP:
                rows[7]=rows[7]|0b0000000000000001;
                break;
            case fabgl::VK_DOWN:
                rows[7]=rows[7]|0b0000000000000010;
                break;
            case fabgl::VK_LEFT:
                rows[7]=rows[7]|0b0000000000000100;
                break;
            case fabgl::VK_RIGHT:
                rows[7]=rows[7]|0b0000000000001000;
                break;
            case fabgl::VK_SPACE:
                rows[7]=rows[7]|0b0000000000010000;
                break;
            case fabgl::VK_LSHIFT:
            case fabgl::VK_RSHIFT:
                rows[7]=rows[7]|0b0000000000100000;
                break;
        }
    }
    else
    {
        // down, make bits 0
        switch (vk)
        {
            case fabgl::VK_UP:
                rows[7]=rows[7]&0b1111111111111110;
                break;
            case fabgl::VK_DOWN:
                rows[7]=rows[7]&0b1111111111111101;
                break;
            case fabgl::VK_LEFT:
                rows[7]=rows[7]&0b1111111111111011;
                break;
            case fabgl::VK_RIGHT:
                rows[7]=rows[7]&0b1111111111110111;
                break;
            case fabgl::VK_SPACE:
                rows[7]=rows[7]&0b1111111111101111;
                break;
            case fabgl::VK_LSHIFT:
            case fabgl::VK_RSHIFT:
                rows[7]=rows[7]&0b1111111111011111;
                break;
        }
    }
}

uint8_t SG1000_PPI8255::get_row_bits (uint8_t row)
{
    if (row<sizeof(rows))
        return rows[row];

    // error, fallback
    return 0xff; // all bits high, no keypress
}
uint8_t SG1000_PPI8255::get_next_row ()
{
    uint8_t row = current_row;
    current_row++;
    if (current_row>=sizeof(rows))
        current_row = 0;
    return row;
}

PPI8255::PPI8255 ()
{
    portA = portB = portC = control = 0;
    current_row = 0;
}
void PPI8255::write (uint8_t port, uint8_t value)
{
    switch (port)
    {
        case 0: 
            portA=value;
            // hal_printf ("PPI write port A: %02x\r\n",value);
            break;
        case 1: 
            portB=value;
            // hal_printf ("PPI write port B: %02x\r\n",value);
            break;
        case 2: 
            portC=value;
            // hal_printf ("PPI write port C: %02x\r\n",value);
            break;
        case 3: 
            control=value;
            // hal_printf ("PPI write control port: %02x\r\n",value);
            break;
    }
}