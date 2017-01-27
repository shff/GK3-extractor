// Types

typedef void*(*handler)(char*);

// Enumerations

typedef enum { SitnSpin = 0x00, Yield = 0x01, CallSysFunctionV = 0x02,
  CallSysFunctionI = 0x03, CallSysFunctionF = 0x04, CallSysFunctionS = 0x05,
  Branch = 0x06, BranchGoto = 0x07, BranchIfZero = 0x08, BeginWait = 0x09,
  EndWait = 0x0A, ReturnV = 0x0B, StoreI = 0x0D, StoreF = 0x0E, StoreS = 0x0F,
  LoadI = 0x10, LoadF = 0x11, LoadS = 0x12, PushI = 0x13, PushF = 0x14,
  PushS = 0x15, Pop = 0x16, AddI = 0x17, AddF = 0x18, SubtractI = 0x19,
  SubtractF = 0x1A, MultiplyI = 0x1B, MultiplyF = 0x1C, DivideI = 0x1D,
  DivideF = 0x1E, NegateI = 0x1F, NegateF = 0x20, IsEqualI = 0x21,
  IsEqualF = 0x22, NotEqualI = 0x23, NotEqualF = 0x24, IsGreaterI = 0x25,
  IsGreaterF = 0x26, IsLessI = 0x27, IsLessF = 0x28, IsGreaterEqualI = 0x29,
  IsGreaterEqualF = 0x2A, IsLessEqualI = 0x2B, IsLessEqualF = 0x2C,
  IToF = 0x2D, FToI = 0x2E, And = 0x30, Or = 0x31, Not = 0x32,
  GetString = 0x33, DebugBreakpoint = 0x34 } shp_opcode;

// Math Structures

typedef struct __attribute__((packed))
{
  float x, y, z;
} vertice;

typedef struct __attribute__((packed))
{
  float u, v;
} coord;

typedef struct __attribute__((packed))
{
  unsigned short a, b, c, dummy;
} triangle;

// Intermediate Data Structures

typedef struct
{
  unsigned int offset;

  unsigned int count;
  unsigned int capacity;
  struct brn_data_file
  {
    char name[40];
    unsigned int barn;
    unsigned int compression;
    unsigned int offset;
    unsigned int size;
  }* files;
  struct brn_data_barn
  {
    char name[20];
  }* barns;
} brn_data;

typedef struct
{
  unsigned int width;
  unsigned int height;
  unsigned short* content;
} bmp_data;

typedef struct
{
  unsigned int count;
  bmp_data* maps;
} mul_data;

typedef struct
{
  unsigned int vertice_count;
  unsigned int indice_count;
  unsigned int model_count;
  unsigned int surface_count;
  vertice* vertices;
  vertice* normals;
  coord* coords;
  struct bsp_data_triangle
  {
    unsigned short a, b, c, surface_index;
  }* indices;
  struct bsp_data_model
  {
    char name[32];
  }* models;
  struct bsp_data_surface
  {
    unsigned int model_index;
    char texture_name[32];
  }* surfaces;
} bsp_data;

typedef struct
{
  unsigned int mesh_count;
  struct mod_data_mesh
  {
    unsigned int section_count;
    float transform[12];
    struct mod_data_section
    {
      char texture_file[32];
      unsigned int vertice_count;
      unsigned int triangle_count;

      vertice* vertices;
      vertice* normals;
      coord* coords;
      triangle* triangles;
    }* sections;
  }* meshes;
} mod_data;

typedef struct
{
  unsigned int frame_count;
  struct act_data_frame
  {
    unsigned int mesh_count;
    struct act_data_mesh
    {
      float transform[12];
      float bounding_box[6];
      unsigned int section_count;
      struct act_data_section
      {
        unsigned int vertice_count;
        vertice* vertices;
      }* sections;
    }* meshes;
  }* frames;
} act_data;

typedef struct
{
  char bsp[20];
  unsigned int model_count;
  struct scn_data_models
  {
    char name[20];
  }* models;
  char skybox[6][20];
} scn_data;

typedef struct
{
  unsigned int import_count;
  unsigned int const_count;
  unsigned int variable_count;
  unsigned int function_count;
  struct shp_data_import
  {
    char name[40];
  }* imports;
  struct shp_data_const
  {
    unsigned int offset;
    char value[40];
  }* consts;
  struct shp_data_variable
  {
    int type;
    int value;
    char name[40];
  }* variables;
  struct shp_data_function
  {
    char name[40];
    unsigned int offset;
    unsigned int op_count;
    unsigned int label_count;
    unsigned int end;
    struct shp_data_op
    {
      unsigned int offset;
      shp_opcode op;
      unsigned int param;
    }* ops;
    struct shp_label
    {
      unsigned int offset;
    }* labels;
  }* functions;
} shp_data;

// Function Prototypes

brn_data* brn_open(char* filename, int is_main);
void* brn_extract(brn_data* data, char* filename, void*(file_handler)(char*));
void brn_close(brn_data* data);

bmp_data* bmp_handler(char* content);
void bmp_close(bmp_data* data);

mul_data* mul_handler(char* content);
void mul_close(mul_data* data);

bsp_data* bsp_handler(char* content);
void bsp_close(bsp_data* data);

mod_data* mod_handler(char* content);
void mod_close(mod_data* data);

act_data* act_handler(char* content);
void act_close(act_data* data);

scn_data* scn_handler(char* content);
void scn_close(scn_data* data);

shp_data* shp_handler(char* content);
void shp_close(shp_data* data);

void* ini_handler(char* content);
