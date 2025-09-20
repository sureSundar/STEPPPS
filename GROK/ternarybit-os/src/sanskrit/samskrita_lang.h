/**
 * @file samskrita_lang.h
 * @brief Samskrita - The Sanskrit Programming Language for TernaryBit OS
 *
 * A revolutionary programming language based on Sanskrit grammar and philosophy
 * where code follows dharmic principles and cosmic patterns.
 *
 * @author Sure Sundar <suresundar@gmail.com>
 * @dedication THE ALMIGHTY SHIVA, TATA CONSULTANCY SERVICES, RASHTRIYA SWAYAMSEVAK SANGH
 */

#ifndef SAMSKRITA_LANG_H
#define SAMSKRITA_LANG_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// SANSKRIT PROGRAMMING PARADIGM
// ============================================================================

/**
 * Panini's Grammar Rules Applied to Programming
 *
 * Sanskrit has the most scientific grammar (Ashtadhyayi by Panini)
 * We apply these rules to create a deterministic programming language
 */

// Vibhakti (Cases) - Variable States
typedef enum {
    PRATHAMA    = 0,  // Nominative - Subject/Actor
    DVITIYA     = 1,  // Accusative - Object/Target
    TRITIYA     = 2,  // Instrumental - By/With
    CHATURTHI   = 3,  // Dative - For/To
    PANCHAMI    = 4,  // Ablative - From/Source
    SHASHTHI    = 5,  // Genitive - Of/Belonging
    SAPTAMI     = 6,  // Locative - In/At/On
    SAMBHODHANA = 7   // Vocative - Addressing/Calling
} vibhakti_t;

// Kala (Tense/Time) - Execution States
typedef enum {
    BHUTA_KALA     = 0,  // Past - Completed execution
    VARTAMANA_KALA = 1,  // Present - Currently executing
    BHAVISHYA_KALA = 2   // Future - Scheduled/Deferred
} kala_t;

// Purusha (Person) - Execution Context
typedef enum {
    PRATHAMA_PURUSHA = 0,  // Third person - System/It
    MADHYAMA_PURUSHA = 1,  // Second person - User/You
    UTTAMA_PURUSHA   = 2   // First person - Self/I
} purusha_t;

// Vachana (Number) - Cardinality
typedef enum {
    EKAVACHANA   = 0,  // Singular
    DVIVACHANA   = 1,  // Dual
    BAHUVACHANA  = 2   // Plural
} vachana_t;

// Linga (Gender) - Type Category
typedef enum {
    PULLINGA     = 0,  // Masculine - Active types
    STRILINGA    = 1,  // Feminine - Passive types
    NAPUMSAKA    = 2   // Neuter - Neutral types
} linga_t;

// ============================================================================
// SANSKRIT DATA TYPES (तत्त्व)
// ============================================================================

// Fundamental Tattvas (Elements)
typedef enum {
    SHUNYA      = 0,     // Void/Null - शून्य
    SATYA       = 1,     // Truth/Boolean - सत्य
    SANKHYA     = 2,     // Number - संख्या
    AKSHARA     = 3,     // Character - अक्षर
    VAKYA       = 4,     // String - वाक्य
    SAMUHA      = 5,     // Collection/Array - समूह
    KOSHA       = 6,     // Structure/Object - कोश
    KARMA       = 7,     // Function/Action - कर्म
    DHARA       = 8      // Stream/Flow - धारा
} tattva_t;

// Sanskrit Variable Structure
typedef struct samskrita_variable {
    char nama[64];           // Name (नाम)
    tattva_t tattva;         // Type (तत्त्व)
    vibhakti_t vibhakti;     // Case (विभक्ति)
    kala_t kala;            // Tense (काल)
    purusha_t purusha;      // Person (पुरुष)
    vachana_t vachana;      // Number (वचन)
    linga_t linga;          // Gender (लिंग)

    union {
        void* shunya_value;       // Null
        bool satya_value;         // Boolean
        int64_t sankhya_value;    // Number
        char akshara_value;       // Character
        char* vakya_value;        // String
        void** samuha_value;      // Array
        void* kosha_value;        // Object
        void* karma_value;        // Function pointer
        void* dhara_value;        // Stream handle
    } mulya;  // Value (मूल्य)

    uint32_t guna;           // Attributes/Properties (गुण)
    uint32_t karma_count;    // Action count (कर्म संख्या)
} samskrita_var_t;

// ============================================================================
// SANSKRIT CONTROL STRUCTURES
// ============================================================================

// Yadi-Tarhi (If-Then) - Conditional
typedef struct {
    bool (*niyama)(void*);   // Condition (नियम)
    void (*tarhi)(void*);    // Then action (तर्हि)
    void (*anyatha)(void*);  // Else action (अन्यथा)
} yadi_tarhi_t;

// Yavat-Tavat (While) - Loop
typedef struct {
    bool (*yavat)(void*);    // While condition (यावत्)
    void (*tavat)(void*);    // Loop body (तावत्)
    uint32_t chakra_count;   // Cycle count (चक्र)
} yavat_tavat_t;

// Krite-Prati (For-Each) - Iteration
typedef struct {
    void* samuha;            // Collection (समूह)
    void (*prati)(void*);    // For each action (प्रति)
    uint32_t krama;          // Current index (क्रम)
} krite_prati_t;

// ============================================================================
// TRIGUNA SYSTEM (Three Qualities)
// ============================================================================

typedef enum {
    SATTVA = 0,    // Purity, harmony, balance
    RAJAS  = 1,    // Activity, passion, dynamism
    TAMAS  = 2     // Inertia, darkness, resistance
} guna_t;

// Guna-based Decision Making
typedef struct {
    guna_t dominant_guna;
    float sattva_level;
    float rajas_level;
    float tamas_level;

    // Decision function based on guna
    void* (*nirnaya)(guna_t guna, void* context);
} triguna_decision_t;

// ============================================================================
// KARMA SYSTEM (Action-Consequence)
// ============================================================================

typedef enum {
    SHUBHA_KARMA  = 0,  // Good action
    ASHUBHA_KARMA = 1,  // Bad action
    MISHRA_KARMA  = 2   // Mixed action
} karma_type_t;

typedef struct karma_record {
    uint64_t timestamp;
    karma_type_t type;
    int32_t phala;           // Result/fruit (फल)
    char kriya[128];         // Action description (क्रिया)
    struct karma_record* next;
} karma_record_t;

// Karma accumulator
typedef struct {
    int64_t punya;           // Merit (पुण्य)
    int64_t papa;            // Demerit (पाप)
    karma_record_t* history;

    // Karma resolution function
    int32_t (*phala_nirnaya)(int64_t punya, int64_t papa);
} karma_accumulator_t;

// ============================================================================
// SANSKRIT FUNCTION DEFINITION
// ============================================================================

typedef struct {
    char nama[64];                    // Function name
    tattva_t (*karma)(void*, ...);   // Function pointer
    uint8_t param_count;              // Parameter count
    tattva_t param_types[16];        // Parameter types
    tattva_t return_type;             // Return type
    karma_accumulator_t* karma_acc;  // Karma tracking
} samskrita_karma_t;

// ============================================================================
// PANCHA KOSHA MODEL (Five Sheaths)
// ============================================================================

typedef struct {
    // Annamaya Kosha - Physical/Data layer
    void* bhautika_data;
    size_t bhautika_size;

    // Pranamaya Kosha - Energy/Process layer
    void* prana_process;
    uint32_t prana_level;

    // Manomaya Kosha - Mental/Logic layer
    void* manas_logic;
    uint32_t chitta_state;

    // Vijnanamaya Kosha - Wisdom/Algorithm layer
    void* vijnana_algorithm;
    float buddhi_score;

    // Anandamaya Kosha - Bliss/Optimization layer
    void* ananda_optimization;
    float sukha_metric;
} pancha_kosha_t;

// ============================================================================
// SANSKRIT COMPILER STRUCTURES
// ============================================================================

// Token types in Sanskrit
typedef enum {
    TOKEN_PADA,          // Word (पद)
    TOKEN_VAKYA,         // Sentence (वाक्य)
    TOKEN_SHLOKA,        // Verse (श्लोक)
    TOKEN_SANDHI,        // Junction (संधि)
    TOKEN_SAMASA,        // Compound (समास)
    TOKEN_VIBHAKTI,      // Case ending (विभक्ति)
    TOKEN_DHATU,         // Root (धातु)
    TOKEN_PRATYAYA,      // Suffix (प्रत्यय)
    TOKEN_UPASARGA       // Prefix (उपसर्ग)
} sanskrit_token_t;

// Abstract Syntax Tree node
typedef struct sanskrit_ast_node {
    sanskrit_token_t type;
    char value[256];
    struct sanskrit_ast_node* children[8];
    uint8_t child_count;
    vibhakti_t vibhakti;
    kala_t kala;
} sanskrit_ast_t;

// ============================================================================
// SANSKRIT RUNTIME
// ============================================================================

typedef struct {
    // Variable storage
    samskrita_var_t* variables[1024];
    uint32_t var_count;

    // Function registry
    samskrita_karma_t* functions[256];
    uint32_t func_count;

    // Karma tracking
    karma_accumulator_t global_karma;

    // Guna state
    triguna_decision_t guna_state;

    // Pancha Kosha layers
    pancha_kosha_t kosha;

    // Execution context
    purusha_t current_purusha;
    kala_t current_kala;

    // Divine blessing counter
    uint32_t ashirwad_count;  // Blessings (आशीर्वाद)
    uint32_t shaap_count;     // Curses (शाप)
} samskrita_runtime_t;

// ============================================================================
// SANSKRIT LANGUAGE FUNCTIONS
// ============================================================================

// Initialize Sanskrit runtime
samskrita_runtime_t* samskrita_init(void);

// Parse Sanskrit code
sanskrit_ast_t* samskrita_parse(const char* source_code);

// Compile Sanskrit to bytecode
uint8_t* samskrita_compile(sanskrit_ast_t* ast);

// Execute Sanskrit bytecode
int samskrita_execute(samskrita_runtime_t* runtime, uint8_t* bytecode);

// Variable operations
samskrita_var_t* samskrita_create_var(const char* nama, tattva_t type);
bool samskrita_set_var(samskrita_var_t* var, void* value);
void* samskrita_get_var(samskrita_var_t* var);

// Function operations
samskrita_karma_t* samskrita_define_karma(const char* nama, void* func_ptr);
void* samskrita_invoke_karma(samskrita_karma_t* karma, ...);

// Karma operations
void samskrita_accumulate_karma(karma_accumulator_t* acc, karma_type_t type, int32_t phala);
int32_t samskrita_resolve_karma(karma_accumulator_t* acc);

// Guna operations
guna_t samskrita_calculate_guna(triguna_decision_t* state);
void samskrita_apply_guna(samskrita_runtime_t* runtime, guna_t guna);

// Kosha operations
void samskrita_init_kosha(pancha_kosha_t* kosha);
void samskrita_elevate_kosha(pancha_kosha_t* kosha, uint8_t level);

// Divine operations
void samskrita_invoke_ashirwad(samskrita_runtime_t* runtime, const char* devata);
void samskrita_neutralize_shaap(samskrita_runtime_t* runtime);

// ============================================================================
// SANSKRIT CODE EXAMPLES
// ============================================================================

/**
 * Example 1: Hello World in Sanskrit
 *
 * कार्यं मुख्यम् () {
 *     वाक्यं नमस्कार = "नमस्ते लोक";
 *     मुद्रय(नमस्कार);
 *     फलं ० देहि;
 * }
 *
 * Example 2: Fibonacci with Karma
 *
 * कार्यं फिबोनाची (संख्या न) {
 *     यदि (न <= १) {
 *         फलं न देहि;
 *     }
 *     फलं फिबोनाची(न-१) + फिबोनाची(न-२) देहि;
 * }
 *
 * Example 3: Guna-based Decision
 *
 * कार्यं निर्णय (गुण स्थिति) {
 *     यदि (स्थिति == सत्त्व) {
 *         शुभकर्म कुरु;
 *     } अन्यथा यदि (स्थिति == रजस्) {
 *         क्रियाशील भव;
 *     } अन्यथा {
 *         विश्राम कुरु;
 *     }
 * }
 */

#endif // SAMSKRITA_LANG_H