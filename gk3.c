#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int uncompress(unsigned char* out, unsigned long* out_len, unsigned char* in, unsigned long in_len);

#define freadb(dest, size, count, stream) { memcpy(dest, stream + offset, size * count); offset += size * count; }

// Types

typedef void*(*handler)(char*);

// GK3 Structures

typedef struct
{
  char magic[8]; // GK3!Barn
  unsigned short minor_version;
  unsigned short major_version;
  unsigned short minor_header_version;
  unsigned short major_header_version;
  unsigned int total_size;
  unsigned int directory_offset;
} brn_header;

typedef struct
{
  char type[4]; // DDir or Data
  unsigned short minor_version;
  unsigned short major_version;
  unsigned int random_number;
  unsigned int checksum;
  unsigned int directory_size;
  unsigned int header_offset;
  unsigned int data_offset;
} brn_directory_header;

typedef struct
{
  char barn[32];
  char id[4];
  char description[40];
  unsigned int random_number;
  unsigned int file_count;
} brn_ddir_header;

typedef struct __attribute__((packed))
{
  unsigned int size;
  unsigned int offset;
  unsigned int checksum;
  unsigned char type;
  unsigned char compression;
  unsigned char name_length;
} brn_file_header;

typedef struct
{
  char magic[4];
  unsigned int version;
  unsigned int size;
  unsigned int root_index;
  unsigned int model_count;
  unsigned int vertice_count;
  unsigned int texture_coord_count;
  unsigned int vertice_indice_count;
  unsigned int texture_indice_count;
  unsigned int surface_count;
  unsigned int plane_count;
  unsigned int node_count;
  unsigned int polygon_count;
} bsp_header;

typedef struct
{
  char name[32];
} bsp_model;

typedef struct
{
  float bounding_x;
  float bounding_y;
  float bounding_z;
  float bounding_radius;
  float chrome_value;
  float chrome_grazing;
  unsigned int chrome_color;
  unsigned int indice_count;
  unsigned int triangle_count;
} bsp_surface_ext;

typedef struct
{
  unsigned int model_index;
  char texture_name[32];
  float u_offset, v_offset;
  float u_scale, v_scale;
  unsigned short size1, size2;
  int flags;
} bsp_surface;

typedef struct __attribute__((packed))
{
  unsigned short a, b, c;
} bsp_triangle;

typedef struct
{
  char magic[4]; // LDOM
  unsigned char minor_version;
  unsigned char major_version;
  unsigned short unknown1;
  unsigned int mesh_count;
  unsigned int size;
  float lod_distance;
  unsigned int unknown2;
} mod_header;

typedef struct
{
  char magic[4]; // HSEM
  float transform[12];
  unsigned int section_count;
  float bounding_box[6];
} mod_mesh_header;

typedef struct
{
  char magic[4]; // PRGM
  char texture_file[32];

  unsigned int color;
  unsigned int smooth;
  unsigned int vertice_count;
  unsigned int triangle_count;
  unsigned int lod_count;
  unsigned int unknown2;
} mod_section_header;

typedef struct
{
  char magic[4]; // KDOL
  unsigned int count1;
  unsigned int count2;
  unsigned int count3;
} mod_lod_header;

typedef struct
{
  char magic[4];
  unsigned short major_version;
  unsigned short minor_version;
  unsigned int frame_count;
  unsigned int mesh_count;
  unsigned int data_size;
  char model_name[32];
} act_header;

typedef struct __attribute__((packed))
{
  unsigned short mesh_index;
  unsigned int data_size;
} act_frame;

typedef struct __attribute__((packed))
{
  unsigned char type;
  unsigned int data_size;
} act_action;

typedef struct __attribute__((packed))
{
  unsigned short group_index;
  unsigned short vertice_count;
} act_group;

typedef struct __attribute__((packed))
{
  char magic[4];
  unsigned short height;
  unsigned short width;
} bmp_header;

typedef struct __attribute__((packed))
{
  char magic[4];
  unsigned int count;
} mul_header;

typedef struct
{
  char magic[8]; // GK3Sheep
  unsigned int unknown_a;
  unsigned int extra_offset;
  unsigned int data_offset;
  unsigned int data_size;
  unsigned int section_count;
} shp_header;

typedef struct
{
  char type[12]; // SysImports, StringConsts, Variables, Functions, Code
  unsigned int extra_offset;
  unsigned int data_offset;
  unsigned int data_size;
  unsigned int data_count;
} shp_section_header;

typedef struct __attribute__((packed))
{
  char magic[2];
  unsigned int size;
  unsigned short reserved1;
  unsigned short reserved2;
  unsigned int offset;
  unsigned int header_size;
  unsigned int width;
  unsigned int height;
  unsigned short color_planes;
  unsigned short bit_depth;
  unsigned int compression;
  unsigned int image_size;
  unsigned int horizontal_resolution;
  unsigned int vertical_resolution;
  unsigned int color_pallete;
  unsigned int used_collors;
} bmp_file_header;

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
    unsigned int a, b, c, surface_index;
  }* indices;
  struct bsp_data_model
  {
    char name[32];
    int duplicate;
    int surface_count_cache;
  }* models;
  struct bsp_data_surface
  {
    unsigned int model_index;
    char texture_name[32];
    int flags;
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
  char model_name[32];
  unsigned int frame_count;
  struct act_data_frame
  {
    unsigned int mesh_count;
    struct act_data_mesh
    {
      int has_transform;
      float transform[12];
      float bounding_box[6];
      unsigned int section_count;
      struct act_data_section
      {
        unsigned int vertice_count;
        int type;
        vertice* vertices;
      }* sections;
    }* meshes;
  }* frames;
} act_data;

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

typedef struct
{
  char bsp[64];
} scn_data;

typedef struct {
  unsigned int frame_count;
  unsigned int frame_rate;
  unsigned int fade_cursor_delay;
  unsigned int allow_fading;
  char sprite_name[64];
  char alpha_channel[64];
} cur_data;

// Utility Functions

float S1I7F8(unsigned short n)
{
  return (((n & 0x8000) != 0) ? -1 : 1) * (((n & 0xff) / 256.0f) + ((n & 0x7fff) >> 8));
}

float S1I2F5(unsigned char n)
{
  return (((n & 0x80) != 0) ? -1 : 1) * (((n & 0x1f) / 32.0f) + ((n & 0x7f) >> 5));
}

vertice normalize(vertice v)
{
  float mag = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
  v.x /= mag;
  v.y /= mag;
  v.z /= mag;
  return v;
}

vertice get_normal(vertice a, vertice b, vertice c)
{
  vertice n;
  n.x = (a.y - b.y) * (a.z + b.z) + (b.y - c.y) * (b.z + c.z) + (c.y - a.y) * (c.z + a.z);
  n.y = (a.z - b.z) * (a.x + b.x) + (b.z - c.z) * (b.x + c.x) + (c.z - a.z) * (c.x + a.x);
  n.z = (a.x - b.x) * (a.y + b.y) + (b.x - c.x) * (b.y + c.y) + (c.x - a.x) * (c.y + a.y);
  return normalize(n);
}

vertice transform(float m[12], vertice v)
{
  vertice newv;
  newv.x = m[0] * v.x + m[3] * v.y + m[6] * v.z;
  newv.y = m[1] * v.x + m[4] * v.y + m[7] * v.z;
  newv.z = m[2] * v.x + m[5] * v.y + m[8] * v.z;
  return newv;
}

// Decompressors

int lzo(const unsigned char* in, unsigned int in_len, unsigned char* out, unsigned int* out_len)
{
  #define UA_COPY4(dd,ss)     (* (unsigned int*) (void *) (dd) = * (const unsigned int*) (const void *) (ss))
  #define UA_COPY8(dd,ss)     (* (unsigned long int*) (void *) (dd) = * (const unsigned long int*) (const void *) (ss))
  #define UA_GET_LE16(ss)     (* (const unsigned short int*) (const void *) (ss))

  *out_len = 0;

  unsigned char* op = out;
  const unsigned char* ip = in;
  const unsigned char* m_pos;
  const unsigned char* const ip_end = in + in_len;

  unsigned int t;
  if (*ip > 17)
  {
    t = *ip++ - 17;
    if (t < 4)
      goto match_next;
    do *op++ = *ip++; while (--t > 0);
    goto first_literal_run;
  }

  for (;;)
  {
    t = *ip++;
    if (t >= 16)
      goto match;
    /* a literal run */
    if (t == 0)
    {
      while (*ip == 0)
      {
        t += 255;
        ip++;
      }
      t += 15 + *ip++;
    }
    /* copy literals */
    t += 3;
    if (t >= 8) do
    {
      UA_COPY8(op,ip);
      op += 8; ip += 8; t -= 8;
    } while (t >= 8);
    if (t >= 4)
    {
      UA_COPY4(op,ip);
      op += 4; ip += 4; t -= 4;
    }
    if (t > 0)
    {
      *op++ = *ip++;
      if (t > 1) { *op++ = *ip++; if (t > 2) { *op++ = *ip++; } }
    }
first_literal_run:
    t = *ip++;
    if (t >= 16)
        goto match;
    m_pos = op - (1 + 0x0800);
    m_pos -= t >> 2;
    m_pos -= *ip++ << 2;
    *op++ = *m_pos++;
    *op++ = *m_pos++;
    *op++ = *m_pos;
    goto match_done;

    /* handle matches */
    for (;;)
    {
match:
      if (t >= 64)                /* a M2 match */
      {
        m_pos = op - 1;
        m_pos -= (t >> 2) & 7;
        m_pos -= *ip++ << 3;
        t = (t >> 5) - 1;

        goto copy_match;
      }
      else if (t >= 32)           /* a M3 match */
      {
        t &= 31;
        if (t == 0)
        {
          while (*ip == 0)
          {
            t += 255;
            ip++;
          }
          t += 31 + *ip++;
        }
        m_pos = op - 1;
        m_pos -= UA_GET_LE16(ip) >> 2;
        ip += 2;
      }
      else if (t >= 16)           /* a M4 match */
      {
        m_pos = op;
        m_pos -= (t & 8) << 11;
        t &= 7;
        if (t == 0)
        {
          while (*ip == 0)
          {
            t += 255;
            ip++;
          }
          t += 7 + *ip++;
        }
        m_pos -= UA_GET_LE16(ip) >> 2;
        ip += 2;
        if (m_pos == op)
          goto eof_found;
        m_pos -= 0x4000;
      }
      else                            /* a M1 match */
      {
        m_pos = op - 1;
        m_pos -= t >> 2;
        m_pos -= *ip++ << 2;
        *op++ = *m_pos++; *op++ = *m_pos;
        goto match_done;
      }

      /* copy match */
      if (op - m_pos >= 8)
      {
        t += (3 - 1);
        if (t >= 8) do
        {
          UA_COPY8(op,m_pos);
          op += 8; m_pos += 8; t -= 8;
        } while (t >= 8);
        if (t >= 4)
        {
          UA_COPY4(op,m_pos);
          op += 4; m_pos += 4; t -= 4;
        }
        if (t > 0)
        {
          *op++ = m_pos[0];
          if (t > 1) { *op++ = m_pos[1]; if (t > 2) { *op++ = m_pos[2]; } }
        }
      }
      else
      {
copy_match:
        *op++ = *m_pos++; *op++ = *m_pos++;
        do *op++ = *m_pos++; while (--t > 0);
      }

match_done:
      t = ip[-2] & 3;
      if (t == 0)
        break;

    /* copy literals */
match_next:
      *op++ = *ip++;
      if (t > 1) { *op++ = *ip++; if (t > 2) { *op++ = *ip++; } }
      t = *ip++;
    }
  }

eof_found:
  *out_len = ((unsigned int) ((op)-(out)));
  if (ip < ip_end)
    fprintf(stderr, "Input not consumed.\n");
  if (ip > ip_end)
    fprintf(stderr, "Input overrun.\n");
  return 0;
}

// Parsers

brn_data* brn_open(char* filename, int is_main)
{
  brn_data* data = malloc(sizeof(brn_data));
  memset(data, 0, sizeof(brn_data));

  // Open File

  FILE* file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("Can't open barn file: %s.\n", filename);
    exit(1);
  }

  // Read Header

  brn_header h;
  fread(&h, sizeof(brn_header), 1, file);

  if (strncmp(h.magic, "GK3!Barn", 8) != 0)
  {
    printf("Invalid barn file: %s.", filename);
    exit(1);
  }

  // Read the Directory Count

  unsigned int directory_count;
  fseek(file, h.directory_offset, SEEK_SET);
  fread(&directory_count, sizeof(unsigned int), 1, file);

  // Read the Directories Headers

  brn_directory_header dh[directory_count];
  fread(&dh[0], sizeof(brn_directory_header), directory_count, file);

  // Read the offset on the Data Section

  unsigned int data_offset = 0;
  for (unsigned int i = 0; i < directory_count; i++)
  {
    if (strncmp(dh[i].type, "ataD", 4) == 0)
    {
      data_offset = data->offset = dh[i].data_offset;
    }
  }

  // Read DDir Sections

  if (is_main)
  {
    data->barns = malloc(sizeof(struct brn_data_barn) * directory_count);
    memset(data->barns, 0, sizeof(struct brn_data_barn) * directory_count);

    for (unsigned int i = 0; i < directory_count; i++)
    {
      if (strncmp(dh[i].type, "riDD", 4) == 0)
      {
        brn_ddir_header ddh;
        fseek(file, dh[i].header_offset, SEEK_SET);
        fread(&ddh, sizeof(brn_ddir_header), 1, file);

        // If it's a child barn, cache the data offset

        if (ddh.barn[0] != 0)
        {
          brn_data* datab = brn_open(&ddh.barn[0], 0);
          data_offset = datab->offset;
          free(datab);
        }

        strncpy(data->barns[i].name, (ddh.barn[0] ? ddh.barn : filename), 40);

        // Read file information

        fseek(file, dh[i].data_offset, SEEK_SET);
        for (unsigned int j = 0; j < ddh.file_count; j++)
        {
          brn_file_header fh;
          fread(&fh, sizeof(brn_file_header), 1, file);

          // Store File Info

          if (data->count + 1 > data->capacity)
            data->files = realloc(data->files, sizeof(struct brn_data_file) * (data->capacity += 512));

          fread(&data->files[data->count].name, sizeof(char), fh.name_length + 1, file);
          data->files[data->count].barn = i;
          data->files[data->count].compression = fh.compression;
          data->files[data->count].offset = fh.offset + data_offset;
          data->files[data->count].size = fh.size;

          data->count++;
        }
      }
    }
  }

  fclose(file);

  return data;
}

void* brn_extract(brn_data* data, char* filename, void*(file_handler)(char*))
{
  // Locate File

  for (unsigned int i = 0; i < data->count; i++)
  {
    if (strcmp(data->files[i].name, filename) == 0)
    {
      struct brn_data_file f = data->files[i];

      // Open Barn

      FILE* file = fopen(data->barns[f.barn].name, "r");
      if (file == NULL)
      {
        printf("Can't open barn file: %s.\n", data->barns[f.barn].name);
        return NULL;
      };

      // Read Content

      unsigned long size = f.size;

      char* buffer = malloc(size + 1);
      fseek(file, f.offset, SEEK_SET);
      fread(buffer, sizeof(char), size, file);

      // Decompress

      if (f.compression > 0)
      {
        // unsigned long realsize = *((int*)buffer);
        unsigned long realsize = 0;
        memcpy(&realsize, buffer, sizeof(unsigned int));
        char* buffer2 = malloc(realsize + 1);

        if (f.compression == 1)
          uncompress((unsigned char*)buffer2, (unsigned long*)&realsize, (unsigned char*)&buffer[8], size + 8);
        else if (f.compression == 2)
          lzo((unsigned char*)&buffer[8], (unsigned int)(size - 8), (unsigned char*)buffer2, (unsigned int*)&realsize);

        free(buffer);
        buffer = buffer2;
        size = realsize;
      }

      // Call Handler

      buffer[size] = 0;
      void* ret = NULL;
      if (file_handler)
      {
        ret = file_handler(buffer);
      }
      else
      {
        FILE* out = fopen(filename, "w");
        fwrite(buffer, size, 1, out);
        fclose(out);
      }

      // Free all buffers

      free(buffer);
      fclose(file);

      return ret;
    }
  }

  fprintf(stderr, "Can't find file %s\n", filename);

  return NULL;
}

void brn_close(brn_data* data)
{
  free(data->barns);
  free(data->files);
  free(data);
}

bmp_data* bmp_handler(char* content)
{
  unsigned int offset = 0;
  bmp_data* data = malloc(sizeof(bmp_data));

  // Read Header

  bmp_header h;
  freadb(&h, sizeof(bmp_header), 1, content);

  if (strncmp(h.magic, "61nM", 4) != 0)
  {
    fprintf(stderr, "Invalid BMP Header\n");
    return NULL;
  }

  data->height = h.height;
  data->width = h.width;

  // Read Contents

  data->content = malloc(sizeof(unsigned short) * h.height * (h.width + h.width % 4));
  freadb(data->content, sizeof(unsigned short) * h.height * (h.width + h.width % 4), 1, content);

  return data;
}

void bmp_close(bmp_data* data)
{
  if (data == NULL) return;
  free(data->content);
  free(data);
}

mul_data* mul_handler(char* content)
{
  unsigned int offset = 0;
  mul_data* data = malloc(sizeof(mul_data));

  // Read Header

  mul_header h;
  freadb(&h, sizeof(mul_header), 1, content);

  if (strncmp(h.magic, "TLUM", 4) != 0)
  {
    fprintf(stderr, "Invalid Lightmap file\n");
    return NULL;
  }

  data->count = h.count;

  // Read Content

  data->maps = malloc(h.count * sizeof(bmp_data));
  for (unsigned int i = 0; i < h.count; i++)
  {
    // Read Bitmap Header

    bmp_file_header bh;
    freadb(&bh, sizeof(bmp_file_header), 1, content);

    if (strncmp(bh.magic, "BM", 2) != 0)
    {
      fprintf(stderr, "Invalid BMP Header\n");
      return NULL;
    }

    data->maps[i].width = bh.width;
    data->maps[i].height = bh.height;

    // Read Bitmap Contents

    data->maps[i].content = malloc(bh.image_size);
    freadb(data->maps[i].content, bh.image_size, 1, content);
  }

  return data;
}

void mul_close(mul_data* data)
{
  for (unsigned int i = 0; i < data->count; i++)
    free(data->maps[i].content);
  free(data->maps);
  free(data);
}

bsp_data* bsp_handler(char* content)
{
  unsigned int offset = 0;
  bsp_data* data = malloc(sizeof(bsp_data));
  memset(data, 0, sizeof(bsp_data));

  // Load Header

  bsp_header h;
  freadb(&h, sizeof(bsp_header), 1, content);

  if (strncmp(h.magic, "NECS", 4) != 0)
  {
    fprintf(stderr, "Invalid BSP file\n");
    return NULL;
  }

  data->model_count = h.model_count;
  data->surface_count = h.surface_count;
  data->vertice_count = h.vertice_count;

  // Prepare Structures

  bsp_surface surfaces[h.surface_count];

  data->models = malloc(sizeof(struct bsp_data_model) * h.model_count);
  data->surfaces = malloc(sizeof(struct bsp_data_surface) * h.surface_count);
  data->vertices = malloc(sizeof(vertice) * h.vertice_count);
  data->normals = malloc(sizeof(vertice) * h.vertice_count);
  data->coords = malloc(sizeof(coord) * h.texture_coord_count);

  // Load contents - Ignore BSP-tree information

  for (unsigned int i = 0; i < h.model_count; i++)
  {
    freadb(&data->models[i], sizeof(bsp_model), 1, content);
    data->models[i].duplicate = 0;
  }

  freadb(&surfaces, sizeof(bsp_surface), h.surface_count, content);
  offset += h.node_count * sizeof(unsigned short) * 8;
  offset += h.polygon_count * sizeof(unsigned short) * 4;
  offset += h.plane_count * sizeof(float) * 4;
  freadb(data->vertices, sizeof(vertice), h.vertice_count, content);
  freadb(data->coords, sizeof(coord), h.texture_coord_count, content);
  offset += h.vertice_indice_count * sizeof(unsigned short);
  offset += h.texture_indice_count * sizeof(unsigned short);
  offset += h.node_count * sizeof(float) * 4;

  // Cache surface count

  for (unsigned int i = 0; i < h.model_count; i++)
  {
    unsigned int surface_count_cache = 0;
    for (unsigned int j = 0; j < h.surface_count; j++)
    {
      if (surfaces[j].model_index == i)
      {
        surface_count_cache += 1;
      }
    }

    data->models[i].surface_count_cache = surface_count_cache;
  }

  // Copy surfaces

  for (unsigned int i = 0; i < h.surface_count; i++)
  {
    char texture_name[64];
    sprintf(texture_name, "%s.BMP", surfaces[i].texture_name);
    for(int j = 0; (texture_name[j] = (char)toupper(texture_name[j])); j++);

    data->surfaces[i].flags = surfaces[i].flags;
    data->surfaces[i].model_index = surfaces[i].model_index;
    strncpy(data->surfaces[i].texture_name, texture_name, 32);
  }

  // Reverse Z order

  for (unsigned int i = 0; i < h.vertice_count; i++)
  {
    data->coords[i].v = 1.0f - data->coords[i].v;
  }

  for (unsigned int i = 0; i < h.vertice_count; i++)
  {
    data->vertices[i].z = -data->vertices[i].z;
  }

  // Read Non-BSP Indexes and Triangles

  unsigned int capacity = 0;
  data->indice_count = 0;

  if (h.version >= 514)
  {
    for (unsigned short i = 0; i < h.surface_count; i++)
    {
      bsp_surface_ext e;
      freadb(&e, sizeof(bsp_surface_ext), 1, content);

      unsigned short indices[e.indice_count];
      bsp_triangle triangles[e.triangle_count];
      freadb(&indices, sizeof(unsigned short), e.indice_count, content);
      freadb(&triangles, sizeof(bsp_triangle), e.triangle_count, content);

      for (unsigned int j = 0; j < e.triangle_count; j++)
      {
        if (data->indice_count + 1 > capacity)
          data->indices = realloc(data->indices, sizeof(struct bsp_data_triangle) * (capacity += 512));

        // Vertice order is inverted in GK3
        data->indices[data->indice_count].a = indices[triangles[j].c];
        data->indices[data->indice_count].b = indices[triangles[j].b];
        data->indices[data->indice_count].c = indices[triangles[j].a];
        data->indices[data->indice_count].surface_index = i;

        // Calculate Normals

        vertice normal = get_normal(
          data->vertices[indices[triangles[j].a]],
          data->vertices[indices[triangles[j].b]],
          data->vertices[indices[triangles[j].c]]);
        data->normals[indices[triangles[j].a]] = normal;
        data->normals[indices[triangles[j].b]] = normal;
        data->normals[indices[triangles[j].c]] = normal;

        data->indice_count++;
      }
    }
  }

  return (void*)data;
}

void bsp_close(bsp_data* data)
{
  free(data->vertices);
  free(data->normals);
  free(data->coords);
  free(data->indices);
  free(data);
}

mod_data* mod_handler(char* content)
{
  unsigned int offset = 0;
  mod_data* data = malloc(sizeof(mod_data));
  memset(data, 0, sizeof(mod_data));

  // Load Header

  mod_header h;
  freadb(&h, sizeof(mod_header), 1, content);

  if (strncmp(h.magic, "LDOM", 4) != 0)
  {
    fprintf(stderr, "Invalid model file.\n");
    return NULL;
  }

  if (h.major_version == 1 && h.minor_version == 9)
    offset += sizeof(unsigned int) * 6;

  data->mesh_count = h.mesh_count;

  // Load MESH Sections

  data->meshes = malloc(sizeof(struct mod_data_mesh) * h.mesh_count);

  for (unsigned int i = 0; i < h.mesh_count; i++)
  {
    // Load MESH header

    mod_mesh_header mh;
    freadb(&mh, sizeof(mod_mesh_header), 1, content);

    if (strncmp(mh.magic, "HSEM", 4) != 0)
    {
      fprintf(stderr, "Invalid model mesh section.\n");
      return NULL;
    }

    memcpy(data->meshes[i].transform, mh.transform, sizeof(float) * 12);

    // Load MGRP Sections

    data->meshes[i].section_count = mh.section_count;
    data->meshes[i].sections = malloc(sizeof(struct mod_data_section) * mh.section_count);

    for (unsigned int j = 0; j < mh.section_count; j++)
    {
      // Read MGRP Header

      mod_section_header sh;
      freadb(&sh, sizeof(mod_section_header), 1, content);

      if (strncmp(sh.magic, "PRGM", 4) != 0)
      {
        fprintf(stderr, "Invalid model mesh group section.\n");
        return NULL;
      }

      char texture_file[64];
      sprintf(texture_file, "%s.BMP", sh.texture_file);
      strncpy(data->meshes[i].sections[j].texture_file, texture_file, 32);
      data->meshes[i].sections[j].triangle_count = sh.triangle_count;

      // Read Vertice and Triangle Data

      data->meshes[i].sections[j].vertice_count = sh.vertice_count;
      data->meshes[i].sections[j].triangle_count = sh.triangle_count;

      data->meshes[i].sections[j].vertices = malloc(sizeof(vertice) * sh.vertice_count);
      data->meshes[i].sections[j].normals = malloc(sizeof(vertice) * sh.vertice_count);
      data->meshes[i].sections[j].coords = malloc(sizeof(coord) * sh.vertice_count);
      data->meshes[i].sections[j].triangles = malloc(sizeof(triangle) * sh.triangle_count);

      freadb(data->meshes[i].sections[j].vertices, sizeof(vertice), sh.vertice_count, content);
      freadb(data->meshes[i].sections[j].normals, sizeof(vertice), sh.vertice_count, content);
      freadb(data->meshes[i].sections[j].coords, sizeof(coord), sh.vertice_count, content);
      freadb(data->meshes[i].sections[j].triangles, sizeof(triangle), sh.triangle_count, content);

      // Convert UV coordinates

      for (unsigned int k = 0; k < data->meshes[i].sections[j].vertice_count; k++)
      {
        data->meshes[i].sections[j].coords[k].v = 1.0f - data->meshes[i].sections[j].coords[k].v;
      }

      // Read LODK Sections - Ignore

      for (unsigned int k = 0; k < sh.lod_count; k++)
      {
        mod_lod_header lh;
        freadb(&lh, sizeof(mod_lod_header), 1, content);

        if (strncmp(lh.magic, "KDOL", 4) != 0)
        {
          fprintf(stderr, "Invalid LODK section\n");
          return NULL;
        }

        offset += lh.count1 * 4 * sizeof(unsigned short);
        offset += lh.count2 * 2 * sizeof(unsigned short);
        offset += lh.count3 * 1 * sizeof(unsigned short);
      }
    }
  }

  return data;
}

void mod_close(mod_data* data)
{
  for (unsigned int i = 0; i < data->mesh_count; i++)
  {
    for (unsigned int j = 0; j < data->meshes[i].section_count; j++)
    {
      free(data->meshes[i].sections[j].vertices);
      free(data->meshes[i].sections[j].normals);
      free(data->meshes[i].sections[j].coords);
      free(data->meshes[i].sections[j].triangles);
    }
  }
  for (unsigned int i = 0; i < data->mesh_count; i++)
    free(data->meshes[i].sections);
  free(data->meshes);
  free(data);
}

act_data* act_handler(char* content)
{
  unsigned int offset = 0;
  act_data* data = malloc(sizeof(act_data));
  memset(data, 0, sizeof(act_data));

  // Read Header

  act_header h;
  freadb(&h, sizeof(act_header), 1, content);

  if (strncmp(h.magic, "HTCA", 4) != 0)
  {
    fprintf(stderr, "Invalid animation file.\n");
    return NULL;
  }

  memcpy(data->model_name, h.model_name, 32);
  for (int i = 0; i < 32; i++)
  {
    data->model_name[i] = toupper(data->model_name[i]);
  }

  // Get Frame Offsets

  unsigned int offsets[h.frame_count];
  freadb(&offsets, sizeof(unsigned int), h.frame_count, content);

  // Read Frames

  data->frame_count = h.frame_count;
  data->frames = malloc(sizeof(struct act_data_frame) * h.frame_count);

  for (unsigned int i = 0; i < h.frame_count; i++)
  {
    // Assert position

    if (offsets[i] != offset)
    {
      printf("Incorrect offset in ACT file\n");
      return NULL;
    }

    data->frames[i].mesh_count = h.mesh_count;
    data->frames[i].meshes = malloc(sizeof(struct act_data_mesh) * h.mesh_count);

    // Read Meshes

    for (unsigned int j = 0; j < h.mesh_count; j++)
    {
      act_frame f;
      freadb(&f, sizeof(act_frame), 1, content);

      if (f.mesh_index != j)
      {
        fprintf(stderr, "Invalid animation frame - wrong mesh index.\n");
        return NULL;
      }

      // Read Groups

      data->frames[i].meshes[j].section_count = 0;
      data->frames[i].meshes[j].sections = NULL;
      data->frames[i].meshes[j].has_transform = 0;

      unsigned int end = offset + f.data_size;

      while (offset < end)
      {
        act_action a;
        freadb(&a, sizeof(act_action), 1, content);

        if (a.type == 0 || a.type == 1)
        {
          act_group g;
          freadb(&g, sizeof(act_group), 1, content);
          unsigned int k = g.group_index;

          // Reallocate the section array, if needed

          if (k + 1 > data->frames[i].meshes[j].section_count)
          {
            data->frames[i].meshes[j].sections = realloc(data->frames[i].meshes[j].sections,
              sizeof(struct act_data_section) * ++(data->frames[i].meshes[j].section_count));
          }

          // Allocate the Vertice Array

          data->frames[i].meshes[j].sections[k].type = a.type;
          data->frames[i].meshes[j].sections[k].vertice_count = g.vertice_count;

          // TODO - Find why I'm getting a segmentation error here when I use a proper size :/

          data->frames[i].meshes[j].sections[k].vertices = malloc(sizeof(vertice) * g.vertice_count * 20);

          // Read Vertices

          if (a.type == 0)
          {
            // Uncompressed vertices

            freadb(data->frames[i].meshes[j].sections[k].vertices, sizeof(vertice), g.vertice_count, content);
          }
          else if (a.type == 1)
          {
            // Compressed vertices

            char mask[g.vertice_count / 4 + 1];
            freadb(&mask, sizeof(char), g.vertice_count / 4 + 1, content);

            for (unsigned int l = 0; l < g.vertice_count; l++)
            {
              vertice v = { .x = 0.0f, .y = 0.0f, .z = 0.0f };

              // Decompress

              unsigned int compression = (mask[l / 4] >> ((l & 3) * 2)) & 0x3;
              if (compression == 1)
              {
                unsigned char c[3];
                freadb(&c, sizeof(unsigned char), 3, content);

                v.x = S1I2F5(c[0]);
                v.y = S1I2F5(c[1]);
                v.z = S1I2F5(c[2]);
              }
              else if (compression == 2)
              {
                unsigned short c[3];
                freadb(&c, sizeof(unsigned short), 3, content);

                v.x = S1I7F8(c[0]);
                v.y = S1I7F8(c[1]);
                v.z = S1I7F8(c[2]);
              }
              else if (compression != 0)
              {
                printf("Unknown compression\n");
                exit(1);
              }

              data->frames[i].meshes[j].sections[k].vertices[l] = v;
            }
          }
        }
        else if (a.type == 2)
        {
          // store modification on the transformation matrix

          data->frames[i].meshes[j].has_transform = 1;
          freadb(data->frames[i].meshes[j].transform, sizeof(float), 12, content);
        }
        else if (a.type == 3)
        {
          // TODO store modification on the bounding box

          offset += sizeof(float) * 6;
        }
      }
    }
  }

  return data;
}

void act_close(act_data* data)
{
  if (data)
  {
    for (unsigned int i = 0; i < data->frame_count; i++)
    {
      for (unsigned int j = 0; j < data->frames[i].mesh_count; j++)
      {
        for (unsigned int k = 0; k < data->frames[i].meshes[j].section_count; k++)
        {
          free(data->frames[i].meshes[j].sections[k].vertices);
        }
        free(data->frames[i].meshes[j].sections);
      }
      free(data->frames[i].meshes);
    }
    free(data->frames);
    free(data);
  }
}

shp_data* shp_handler(char* content)
{
  unsigned int offset = 0;
  shp_data* data = malloc(sizeof(shp_data));
  memset(data, 0, sizeof(shp_data));

  // Read Header

  shp_header h;
  freadb(&h, sizeof(shp_header), 1, content);

  if (strncmp(h.magic, "GK3Sheep", 8) != 0)
  {
    fprintf(stderr, "Invalid GK3 SHP File");
    return NULL;
  }

  unsigned int offsets[h.section_count];
  freadb(&offsets, sizeof(unsigned int), h.section_count, content);

  // Read Sections

  unsigned int code_offset;
  for (unsigned int i = 0; i < h.section_count; i++)
  {
    shp_section_header sh;
    offset = h.data_offset + offsets[i];
    freadb(&sh, sizeof(shp_section_header), 1, content);

    unsigned int offsetsb[sh.data_count];
    freadb(&offsetsb, sizeof(unsigned int), sh.data_count, content);

    if (strncmp(sh.type, "SysImports", 10) == 0)
    {
      data->import_count = sh.data_count;
      data->imports = malloc(sizeof(struct shp_data_import) * sh.data_count);

      for (unsigned int j = 0; j < sh.data_count; j++)
      {
        offset = h.data_offset + offsets[i] + sh.data_offset + offsetsb[j] + 2;
        strncpy(data->imports[j].name, &content[offset], 40);
      }
    }
    else if (strncmp(sh.type, "StringConsts", 11) == 0)
    {
      data->const_count = sh.data_count;
      data->consts = malloc(sizeof(struct shp_data_const) * sh.data_count);

      for (unsigned int j = 0; j < sh.data_count; j++)
      {
        offset = h.data_offset + offsets[i] + sh.data_offset + offsetsb[j];
        strncpy(data->consts[j].value, &content[offset], 40);
        data->consts[j].offset = offsetsb[j];
      }
    }
    else if (strncmp(sh.type, "Functions", 9) == 0)
    {
      data->function_count = sh.data_count;
      data->functions = malloc(sizeof(struct shp_data_function) * sh.data_count);

      for (unsigned int j = 0; j < sh.data_count; j++)
      {
        memset(&data->functions[j], 0, sizeof(struct shp_data_function));

        offset = h.data_offset + offsets[i] + sh.data_offset + offsetsb[j] + 2;
        strncpy(data->functions[j].name, &content[offset], 40);

        offset += strnlen(data->functions[j].name, 40) + 1 + 2;
        freadb(&data->functions[j].offset, sizeof(unsigned short), 1, content);
      }
    }
    else if (strncmp(sh.type, "Variables", 9) == 0)
    {
      data->variable_count = sh.data_count;
      data->variables = malloc(sizeof(struct shp_data_variable) * sh.data_count);

      for (unsigned int j = 0; j < sh.data_count; j++)
      {
        offset = h.data_offset + offsets[i] + sh.data_offset + offsetsb[j];
        unsigned short str_size;
        freadb(&str_size, sizeof(unsigned short), 1, content);
        freadb(&data->variables[j].name, sizeof(char), str_size + 1, content);
        freadb(&data->variables[j].type, sizeof(unsigned int), 1, content);
        freadb(&data->variables[j].value, sizeof(unsigned int), 1, content);
      }
    }
    else if (strncmp(sh.type, "Code", 4) == 0)
    {
      code_offset = h.data_offset + offsets[i] + sh.data_offset + offsetsb[0];
    }
  }

  // Read Opcodes

  for (unsigned int i = 0; i < data->function_count; i++)
  {
    offset = code_offset + data->functions[i].offset;

    for(unsigned int j = 0, k = 0; ; j++)
    {
      if (j + 1 > k)
        data->functions[i].ops = realloc(data->functions[i].ops, sizeof(struct shp_data_op) * (k += 64));

      data->functions[i].ops[j].offset = offset - code_offset;
      data->functions[i].ops[j].op = 0;
      freadb(&data->functions[i].ops[j].op, sizeof(unsigned char), 1, content);

      unsigned char op = data->functions[i].ops[j].op;
      if (op == CallSysFunctionV || op == CallSysFunctionI || op == BranchIfZero || op == Branch
        || op == BranchGoto || op == StoreI || op == StoreF || op == StoreS || op == LoadI
        || op == LoadF || op == LoadS || op == PushI || op == PushF || op == PushS || op == IToF)
        freadb(&data->functions[i].ops[j].param, sizeof(unsigned int), 1, content);

      data->functions[i].op_count = j;
      data->functions[i].end = offset;

      if (op == ReturnV)
        break;
    }

    data->functions[i].label_count = 0;
    for (unsigned int j = 0, k = 0; j < data->functions[i].op_count; j++)
    {
      if (data->functions[i].ops[j].op == BranchGoto
        && data->functions[i].ops[j].param >= data->functions[i].offset
        && data->functions[i].ops[j].param < data->functions[i].end)
      {
        if (j + 1 > k)
          data->functions[i].labels = realloc(data->functions[i].labels, sizeof(struct shp_label) * (k += 64));

        data->functions[i].labels[data->functions[i].label_count++].offset = data->functions[i].ops[j].param;
      }
    }
  }

  return data;
}

void shp_close(shp_data* data)
{
  for (unsigned int i = 0; i < data->function_count; i++)
    free(data->functions[i].ops), free(data->functions[i].labels);
  free(data->imports);
  free(data->consts);
  free(data->functions);
  free(data->variables);
  free(data);
}

scn_data* scn_handler(char* content)
{
  scn_data* data = malloc(sizeof(scn_data));
  memset(data, 0, sizeof(scn_data));

  // Split line by line
  unsigned int mode = 0;
  char current_light[64];
  for(char *t = content, *s, *line; (line = strtok_r(t, "\n\r", &s)); t = NULL)
  {
    line[strcspn(line, "/")] = 0;               // Remove comments
    for(char* m = s - 2; *--m == 10; *m = 0);   // Right trim
    for(; *line && *line == 10; line++);        // Left trim
    if (!*line) continue;                       // Skip blank lines

    if (strncmp(line, "[MODELS]", 8) == 0)
      mode = 1;
    else if (strncmp(line, "[LIGHTS]", 8) == 0)
      mode = 2;
    else if (strncmp(line, "[Skybox]", 8) == 0)
      mode = 3;
    else if (strncmp(line, "[Light_", 7) == 0)
      mode = 4, strncpy(current_light, line + 7, strcspn(line + 7, "]"));
    else if (mode == 0 && strncmp(line, "bsp=", 4) == 0)
    {
      for(int i = 0; (line[i] = (char)toupper(line[i])); i++);
      strncpy(data->bsp, line + 4, 64);
      sprintf(data->bsp, "%s.BSP", data->bsp);
    }
  }

  return data;
}

void scn_close(scn_data* data)
{
  free(data);
}

cur_data* cur_handler(char* content)
{
  cur_data* data = malloc(sizeof(cur_data));
  memset(data, 0, sizeof(cur_data));
  data->frame_count = 1;

  for(char *t = content, *s, *line; (line = strtok_r(t, "\n\r", &s)); t = NULL)
  {
    line[strcspn(line, ";")] = 0;               // Remove comments
    if (!*line) continue;                       // Skip blank lines

    // TODO: Hotspot, Transparency
    if (strncmp("Allow Fading=yes", line, 16) == 0)
      data->allow_fading = 1;
    else if (strncmp("Alpha Channel", line, 13) == 0)
      strncpy(data->alpha_channel, line + 14, 64);
    else if (strncmp("Sprite Name", line, 11) == 0)
      strncpy(data->sprite_name, line + 12, 64);
    else if (strncmp("Frame Count", line, 11) == 0)
      data->frame_count = atoi(line + 12);
    else if (strncmp("Frame Rate", line, 10) == 0)
      data->frame_rate = atoi(line + 11);
    else if (strncmp("Fade Cursor Delay", line, 17) == 0)
      data->fade_cursor_delay = atoi(line + 18);
  }

  return data;
}

void cur_close(cur_data* data)
{
  free(data);
}

// Writers

void bmp_write(bmp_data* data, char* filename, char* prefix, int color, unsigned int start, unsigned int end)
{
  if (data == NULL) return;

  char filename2[256];
  if (prefix) {
    sprintf(filename2, "%s/%s", prefix, filename);
  } else {
    sprintf(filename2, "%s", filename);
  }

  FILE* f = fopen(filename2, "w");

  // Write Header

  bmp_file_header h =
  {
    .magic = "BM",
    .size = data->height * (data->width * 3 + data->width % 4) + sizeof(bmp_file_header),
    .offset = sizeof(bmp_file_header),
    .header_size = sizeof(bmp_file_header) - 14,
    .width = end == 0 ? data->width : end - start,
    .height = data->height,
    .color_planes = 1,
    .bit_depth = 24,
    .compression = 0,
    .image_size = data->height * (data->width * 3)
  };

  fwrite(&h, sizeof(h), 1, f);

  // Write Contents

  for (int row = (int)data->height - 1; row >= 0; row--)
  {
    for (unsigned int col = 0; col < data->width; col++)
    {
      unsigned short pixel = data->content[(unsigned int)row * (data->width + (data->width % 4) % 2) + col];

      if (end > 0 && (col < start || col >= end)) continue;

      char r = (char)((pixel & 0x001f) * 8);
      char g = (char)(((pixel & 0x07e0) >> 5) * 4);
      char b = (char)(((pixel & 0xf800) >> 11) * 8);

      if (!color) r = g = b = (r + g + b) / 3;

      fwrite(&r, sizeof(char), 1, f);
      fwrite(&g, sizeof(char), 1, f);
      fwrite(&b, sizeof(char), 1, f);
    }
  }

  // Close File

  fclose(f);
}

void bsp_write(bsp_data* data, char* filename, char* model)
{
  // Detect unused indices

  int* used = malloc(data->vertice_count * sizeof(int));
  memset(used, 0, data->vertice_count * sizeof(int));

  for (unsigned int i = 0; i < data->model_count; i++)
  {
    if (data->models[i].duplicate == 1)
      continue;

    if (model && (strcmp(data->models[i].name, model) != 0))
      continue;

    for (unsigned int j = 0; j < data->surface_count; j++)
    {
      if (data->surfaces[j].model_index != i)
        continue;

      // Remove invisible portals between BSPs
      if (data->surfaces[j].flags == 12)
        continue;

      // Remove invisible squares under vehicles
      if (data->surfaces[j].flags & 1 << 6)
        continue;

      // Remove invisible squares under trees
      // TODO: Let's not do it for now! Apparently that removes the floor in some rooms too!
      // if (data->surfaces[j].flags & 1 << 2)
      //   continue;

      // Remove duplicate geometry
      if (data->surfaces[j].flags == 999999)
        continue;

      for (unsigned int k = 0; k < data->indice_count; k++)
      {
        if (data->indices[k].surface_index != j)
          continue;

        used[data->indices[k].a] = 1;
        used[data->indices[k].b] = 1;
        used[data->indices[k].c] = 1;
      }
    }
  }

  // Calculate index shift

  int* shift = malloc(data->vertice_count * sizeof(int));
  memset(shift, 0, data->vertice_count * sizeof(int));

  for (unsigned int i = 0; i < data->vertice_count; i++)
  {
    if (used[i] == 1) continue;

    for (unsigned int j = i + 1; j < data->vertice_count; j++)
    {
      shift[j]++;
    }
  }

  // Output OBJ

  mkdir(filename, S_IRWXU);

  char obj[64], mtl[64];
  sprintf(obj, "%s/%s.OBJ", filename, model ? model : filename);
  sprintf(mtl, "%s/%s.MTL", filename, model ? model : filename);

  FILE* f = fopen(obj, "w");
  FILE* m = fopen(mtl, "w");

  fprintf(f, "mtllib %s.MTL\n", model ? model : filename);

  for (unsigned int i = 0; i < data->vertice_count; i++)
    if (used[i] == 1)
      fprintf(f, "v %f %f %f\n", data->vertices[i].x / 100.f, data->vertices[i].y / 100.f, data->vertices[i].z / 100.f);

  for (unsigned int i = 0; i < data->vertice_count; i++)
    if (used[i] == 1)
      fprintf(f, "vt %f %f\n", data->coords[i].u, data->coords[i].v);

  for (unsigned int i = 0; i < data->vertice_count; i++)
    if (used[i] == 1)
      fprintf(f, "vn %f %f %f\n", data->normals[i].x, data->normals[i].y, data->normals[i].z);

  for (unsigned int i = 0; i < data->model_count; i++)
  {
    if (data->models[i].duplicate == 1)
      continue;

    if (model && strcmp(data->models[i].name, model) != 0)
      continue;

    char texture_name[64] = {0};
    for (unsigned int j = 0; j < data->surface_count; j++)
    {
      if (data->surfaces[j].model_index != i)
        continue;

      // Remove invisible portals between BSPs
      if (data->surfaces[j].flags == 12)
        continue;

      // // Remove invisible squares under vehicles
      if (data->surfaces[j].flags & 1 << 6)
        continue;

      // Remove invisible squares under trees
      // TODO: Let's not do it for now! Apparently that removes the floor in some rooms too!
      // if (data->surfaces[j].flags & 1 << 2)
      //   continue;

      // Remove duplicate geometry
      if (data->surfaces[j].flags == 999999)
        continue;

      if (strncmp(texture_name, data->surfaces[j].texture_name, 64))
      {
        fprintf(f, "g group_%s_TEX_%s # %i\n", data->models[i].name, data->surfaces[j].texture_name, j);
        fprintf(f, "usemtl group_%s\n", data->surfaces[j].texture_name);
        fprintf(m, "newmtl group_%s\n", data->surfaces[j].texture_name);
        fprintf(m, "map_Kd %s\n", data->surfaces[j].texture_name);
        strncpy(texture_name, data->surfaces[j].texture_name, 64);
      }
      else {
        fprintf(f, "# g group_%s_TEX_%s # %i\n", data->models[i].name, data->surfaces[j].texture_name, j);
      }

      for (unsigned int k = 0; k < data->indice_count; k++)
      {
        if (data->indices[k].surface_index != j)
          continue;

        fprintf(f, "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
          data->indices[k].a + 1 - shift[data->indices[k].a],
          data->indices[k].a + 1 - shift[data->indices[k].a],
          data->indices[k].a + 1 - shift[data->indices[k].a],

          data->indices[k].b + 1 - shift[data->indices[k].b],
          data->indices[k].b + 1 - shift[data->indices[k].b],
          data->indices[k].b + 1 - shift[data->indices[k].b],

          data->indices[k].c + 1 - shift[data->indices[k].c],
          data->indices[k].c + 1 - shift[data->indices[k].c],
          data->indices[k].c + 1 - shift[data->indices[k].c]);
      }
    }
  }

  free(shift);
  free(used);

  fclose(m);
  fclose(f);
}

bsp_data* bsp_merge(unsigned int bsp_count, bsp_data** data)
{
  int vertice_index = 0;
  int indice_index = 0;
  int model_index = 0;
  int surface_index = 0;

  bsp_data* new_data = malloc(sizeof(bsp_data));
  memset(new_data, 0, sizeof(bsp_data));

  for (unsigned int i = 0; i < bsp_count; i++)
  {
    new_data->vertice_count = vertice_index + data[i]->vertice_count;
    new_data->indice_count = indice_index + data[i]->indice_count;
    new_data->model_count = model_index + data[i]->model_count;
    new_data->surface_count = surface_index + data[i]->surface_count;

    new_data->vertices = realloc(new_data->vertices, sizeof(vertice) * new_data->vertice_count);
    new_data->coords = realloc(new_data->coords, sizeof(coord) * new_data->vertice_count);
    new_data->normals = realloc(new_data->normals, sizeof(vertice) * new_data->vertice_count);
    new_data->indices = realloc(new_data->indices, sizeof(struct bsp_data_triangle) * new_data->indice_count);
    new_data->models = realloc(new_data->models, sizeof(struct bsp_data_model) * new_data->model_count);
    new_data->surfaces = realloc(new_data->surfaces, sizeof(struct bsp_data_surface) * new_data->surface_count);

    for (unsigned int j = 0; j < data[i]->vertice_count; j++)
    {
      new_data->vertices[j + vertice_index] = data[i]->vertices[j];
      new_data->coords[j + vertice_index] = data[i]->coords[j];
      new_data->normals[j + vertice_index] = data[i]->normals[j];
    }

    for (unsigned int j = 0; j < data[i]->indice_count; j++)
    {
      new_data->indices[j + indice_index].a = data[i]->indices[j].a + vertice_index;
      new_data->indices[j + indice_index].b = data[i]->indices[j].b + vertice_index;
      new_data->indices[j + indice_index].c = data[i]->indices[j].c + vertice_index;
      new_data->indices[j + indice_index].surface_index = data[i]->indices[j].surface_index + surface_index;
    }

    for (unsigned int j = 0; j < data[i]->model_count; j++)
    {
      // Find duplicate model names

      for (unsigned int i_prev = 0; i_prev < i; i_prev++)
      {
        for (unsigned int j_prev = 0; j_prev < data[i_prev]->model_count; j_prev++)
        {
          if (strcmp(data[i]->models[j].name, data[i_prev]->models[j_prev].name) == 0)
          {
            // Mark model with less surfaces as a duplicate

            if (data[i]->models[j].surface_count_cache >= data[i_prev]->models[j_prev].surface_count_cache)
            {
              data[i_prev]->models[j_prev].duplicate = 1;
            }
            else
            {
              data[i]->models[j].duplicate = 1;
            }
          }
        }
      }

      memcpy(&new_data->models[j + model_index], &data[i]->models[j], sizeof(struct bsp_data_model));
    }

    for (unsigned int j = 0; j < data[i]->surface_count; j++)
    {
      memcpy(&new_data->surfaces[j + surface_index], &data[i]->surfaces[j], sizeof(struct bsp_data_surface));
    }

    vertice_index += data[i]->vertice_count;
    indice_index += data[i]->indice_count;
    model_index += data[i]->model_count;
    surface_index += data[i]->surface_count;
  }

  // Deduplicate indices

  for (unsigned int i = 0; i < new_data->vertice_count; i++)
  {
    for (unsigned int i2 = 0; i2 < i; i2++)
    {
      if (new_data->vertices[i].x == new_data->vertices[i2].x
        && new_data->vertices[i].y == new_data->vertices[i2].y
        && new_data->vertices[i].z == new_data->vertices[i2].z
        && new_data->normals[i].x == new_data->normals[i2].x
        && new_data->normals[i].y == new_data->normals[i2].y
        && new_data->normals[i].z == new_data->normals[i2].z
        && new_data->coords[i].u == new_data->coords[i2].u
        && new_data->coords[i].v == new_data->coords[i2].v)
      {
        // Stop using the duplicate version

        for (unsigned int j = 0; j < new_data->indice_count; j++)
        {
          if (new_data->indices[j].a == i2)
            new_data->indices[j].a = i;
          if (new_data->indices[j].b == i2)
            new_data->indices[j].b = i;
          if (new_data->indices[j].c == i2)
            new_data->indices[j].c = i;
        }
      }
    }
  }

  // Get Surface Hashes

  unsigned int *hash = malloc(sizeof(unsigned int) * new_data->indice_count);

  for (unsigned int i = 0; i < new_data->surface_count; i++)
  {
    for (unsigned int j = 0; j < new_data->indice_count; j++)
    {
      if (new_data->indices[j].surface_index != i)
        continue;

      unsigned int indices = new_data->indices[j].a + new_data->indices[j].b + new_data->indices[j].c;
      hash[i] = (((hash[i] & 0xf8000000) << 5) ^ ((hash[i] & 0xf8000000) >> 27)) ^ indices;
    }
  }

  // Mark duplicate surfaces with a flag if it has the same hash as another

  for (unsigned int i = 0; i < new_data->surface_count; i++)
  {
    for (unsigned int i2 = 0; i2 < i; i2++)
    {
      if (i != i2 && strcmp(new_data->surfaces[i].texture_name, new_data->surfaces[i2].texture_name) && hash[i] == hash[i2])
      {
        new_data->surfaces[i].flags = 999999;
      }
    }
  }

  return new_data;
}

void mod_write(mod_data* data, char* filename, char* prefix)
{
  char obj[64], mtl[64];
  sprintf(obj, "%s/%s.OBJ", prefix, filename);
  sprintf(mtl, "%s/%s.MTL", prefix, filename);

  FILE* f = fopen(obj, "w");
  FILE* m = fopen(mtl, "w");

  fprintf(f, "mtllib %s.MTL\n", filename);

  for (unsigned int i = 0; i < data->mesh_count; i++)
  {
    for (unsigned int j = 0; j < data->meshes[i].section_count; j++)
    {
      for (unsigned int k = 0; k < data->meshes[i].sections[j].vertice_count; k++)
      {
        vertice v = transform(data->meshes[i].transform, data->meshes[i].sections[j].vertices[k]);
        fprintf(f, "v %f %f %f\n", v.x, v.y, v.z);
      }
      for (unsigned int k = 0; k < data->meshes[i].sections[j].vertice_count; k++)
      {
        fprintf(f, "vn %f %f %f\n",
          data->meshes[i].sections[j].normals[k].x,
          data->meshes[i].sections[j].normals[k].y,
          data->meshes[i].sections[j].normals[k].z);
      }
      for (unsigned int k = 0; k < data->meshes[i].sections[j].vertice_count; k++)
      {
        fprintf(f, "vt %f %f\n",
          data->meshes[i].sections[j].coords[k].u,
          data->meshes[i].sections[j].coords[k].v);
      }
    }
  }

  int counter = 0;
  int offset = 1;
  for (unsigned int i = 0; i < data->mesh_count; i++)
  {
    for (unsigned int j = 0; j < data->meshes[i].section_count; j++)
    {
      fprintf(f, "g group_%i\n", counter);
      fprintf(f, "usemtl group_%i\n", counter);
      fprintf(m, "newmtl group_%i\n", counter);
      if (data->meshes[i].sections[j].texture_file[0] != 0)
        fprintf(m, "map_Kd %s\n", data->meshes[i].sections[j].texture_file);

      for (unsigned int k = 0; k < data->meshes[i].sections[j].triangle_count; k++)
      {
        triangle t = data->meshes[i].sections[j].triangles[k];
        fprintf(f, "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
          t.a + offset, t.a + offset, t.a + offset,
          t.b + offset, t.b + offset, t.b + offset,
          t.c + offset, t.c + offset, t.c + offset);
      }

      counter++;
      offset += data->meshes[i].sections[j].vertice_count;
    }
  }

  fclose(m);
  fclose(f);
}

void mod_write_act(mod_data* data, act_data* act_data, char* prefix)
{
  for (unsigned int i = 0; i < act_data->frame_count; i++)
  {
    if (data->mesh_count != act_data->frames[i].mesh_count)
    {
      printf("Mesh count mismatch in frame %i - %i (MOD) vs %i (ACT)\n", i, data->mesh_count, act_data->frames[i].mesh_count);
      return;
    }

    for (unsigned int j = 0; j < act_data->frames[i].mesh_count; j++)
    {
      if (act_data->frames[i].meshes[j].has_transform)
      {
        // Apply transformation matrix

        memcpy(data->meshes[j].transform, act_data->frames[i].meshes[j].transform, sizeof(float) * 12);
      }

      if (act_data->frames[i].meshes[j].section_count > data->meshes[j].section_count)
      {
        printf("Session count mismatch in frame %i mesh %i - %i (MOD) vs %i (ACT)\n", i, j, data->meshes[j].section_count, act_data->frames[i].meshes[j].section_count);
        continue;
      }

      for (unsigned int k = 0; k < act_data->frames[i].meshes[j].section_count; k++)
      {
        if (act_data->frames[i].meshes[j].sections[k].vertice_count > data->meshes[j].sections[k].vertice_count)
        {
          // Vertice mismatches happen (eg: GRA_GRALERREADNOTEA.ACT), just ignore
          printf("Vertice count mismatch in frame %i mesh %i section %i - %i (MOD) vs %i (ACT)\n", i, j, k, data->meshes[j].sections[k].vertice_count, act_data->frames[i].meshes[j].sections[k].vertice_count);
          continue;
        }

        for (unsigned int l = 0; l < act_data->frames[i].meshes[j].sections[k].vertice_count; l++)
        {
          if (act_data->frames[i].meshes[j].sections[k].type == 0)
          {
            data->meshes[j].sections[k].vertices[l].x = act_data->frames[i].meshes[j].sections[k].vertices[l].x;
            data->meshes[j].sections[k].vertices[l].y = act_data->frames[i].meshes[j].sections[k].vertices[l].y;
            data->meshes[j].sections[k].vertices[l].z = act_data->frames[i].meshes[j].sections[k].vertices[l].z;
          }
          else if (act_data->frames[i].meshes[j].sections[k].type == 1)
          {
            data->meshes[j].sections[k].vertices[l].x += act_data->frames[i].meshes[j].sections[k].vertices[l].x;
            data->meshes[j].sections[k].vertices[l].y += act_data->frames[i].meshes[j].sections[k].vertices[l].y;
            data->meshes[j].sections[k].vertices[l].z += act_data->frames[i].meshes[j].sections[k].vertices[l].z;
          }
        }
      }
    }

    char filename[64];
    sprintf(filename, "frame_%i", i);
    mod_write(data, filename, prefix);
  }
}

void shp_operator1(char stack[128][128], unsigned int* stack_pos, const char* operator)
{
  char buffer1[512];
  sprintf(buffer1, "%s", stack[(*stack_pos)--]);
  sprintf(stack[++(*stack_pos)], "%s%s", buffer1, operator);
}

void shp_operator2(char stack[128][128], unsigned int* stack_pos, const char* operator)
{
  char buffer1[512];
  char buffer2[512];
  sprintf(buffer1, "%s", stack[(*stack_pos)--]);
  sprintf(buffer2, "%s", stack[(*stack_pos)--]);
  sprintf(stack[++(*stack_pos)], "(%s %s %s)", buffer1, operator, buffer2);
}

#define shp_print(m, ...) fprintf(f, "%.*s" m, tab_offset * 2, "                        ", __VA_ARGS__)

void shp_write(shp_data* data, char* filename)
{
  FILE* f = fopen(filename, "w");
  unsigned int tab_offset = 0;

  // Write Symbols

  shp_print("symbols\n", NULL);
  shp_print("{\n", NULL);
  tab_offset++;
  for (unsigned int i = 0; i < data->variable_count; i++)
  {
    if (data->variables[i].type == 1)
      shp_print("int %s = %i;\n", data->variables[i].name, data->variables[i].value);
    else if (data->variables[i].type == 3)
      shp_print("string %s = \"%s\";\n", data->variables[i].name, data->consts[data->variables[i].value].value);
  }
  tab_offset--;
  shp_print("}\n", NULL);

  // Write Functions

  shp_print("code\n", NULL);
  shp_print("{\n", NULL);
  tab_offset++;
  for (unsigned int i = 0; i < data->function_count; i++)
  {
    unsigned int stack_pos = 0;
    char stack[128][128] = { { 0 } };
    unsigned int wait = 0;
    unsigned int wait_count = 0;
    char wait_buffer[128][128] = { { 0 } };
    unsigned int is_else = 0;
    unsigned int if_stack_pos = 0;
    unsigned int if_stack[128] = { -1 };

    shp_print("%s()\n", data->functions[i].name);
    shp_print("{\n", NULL);
    tab_offset++;

    for (unsigned int j = 0; j < data->functions[i].op_count; j++)
    {
      unsigned int op = data->functions[i].ops[j].op;
      unsigned int param = data->functions[i].ops[j].param;
      unsigned int func_offset = data->functions[i].ops[j].offset;

      if (is_else && (op == BeginWait || op == CallSysFunctionV || op == StoreI || op == StoreF || op == StoreS))
      {
        tab_offset--;
        shp_print("}\n", NULL);
        shp_print("else\n", NULL);
        shp_print("{\n", NULL);
        tab_offset++;
        is_else = 0;
      }

      while (if_stack_pos > 0 && if_stack[if_stack_pos] == func_offset)
      {
        if_stack_pos--;
        tab_offset--;
        shp_print("}\n", NULL);
      }

      for (unsigned int k = 0; k < data->functions[i].label_count; k++)
      {
        if (data->functions[i].labels[k].offset == func_offset)
        {
          shp_print("lbl_%i:\n", func_offset);
          break;
        }
      }

      if (op == CallSysFunctionI)
      {
        int param_count = atoi(stack[stack_pos--]);

        char buffer[128];
        sprintf(buffer, "%s(", data->imports[param].name);
        for (int k = 0; k < param_count; k++)
          sprintf(buffer, "%s%s%s", buffer, stack[stack_pos--], k + 1 == param_count ? "" : ", ");
        sprintf(buffer, "%s)", buffer);

        sprintf(stack[++stack_pos], "%s", buffer);
      }
      if (op == CallSysFunctionV)
      {
        int param_count = atoi(stack[stack_pos--]);

        char buffer[128];
        sprintf(buffer, "%s(", data->imports[param].name);
        for (int k = 0; k < param_count; k++)
          sprintf(buffer, "%s%s%s", buffer, stack[stack_pos--], k + 1 == param_count ? "" : ", ");
        sprintf(buffer, "%s)", buffer);

        if (wait == 1)
          sprintf(wait_buffer[wait_count++], "%s", buffer);
        else
          shp_print("%s;\n", buffer);

        sprintf(stack[++stack_pos], "");
      }
      else if (op == BranchIfZero)
      {
        if (is_else)
        {
          tab_offset--;
          shp_print("}\n", NULL);
          shp_print("else if (%s)\n", stack[stack_pos--]);

          if (if_stack_pos > 0)
            if_stack_pos--;

          is_else = 0;
        }
        else
        {
          shp_print("if (%s)\n", stack[stack_pos--]);
        }
        shp_print("{\n", NULL);
        tab_offset++;

        if_stack[++if_stack_pos] = param;
      }
      else if (op == Branch)
      {
        is_else = 1, if_stack[if_stack_pos] = param;
      }
      else if (op == BranchGoto && param == data->functions[i].end)
      {
        shp_print("return;\n", NULL);
      }
      else if (op == BranchGoto)
      {
        shp_print("goto lbl_%i;\n", param);
      }
      else if (op == BeginWait)
      {
        wait = 1;
      }
      else if (op == EndWait && wait_count == 1)
      {
        shp_print("wait %s;\n", wait_buffer[0]);
        wait = wait_count = wait_buffer[0][0] = 0;
      }
      else if (op == EndWait)
      {
        shp_print("wait\n", NULL);
        shp_print("{\n", NULL);
        tab_offset++;
        for(unsigned int k = 0; k < wait_count; k++)
        {
          shp_print("%s;\n", wait_buffer[k]);
        }
        tab_offset--;
        shp_print("}\n", NULL);

        wait = wait_count = wait_buffer[0][0] = 0;
      }
      else if (op == StoreI || op == StoreF || op == StoreS)
      {
        shp_print("%s = %s;\n", data->variables[param].name, stack[stack_pos--]);
      }
      else if (op == PushS)
      {
        for (unsigned int k = 0; k < data->const_count; k++)
        {
          if (data->consts[k].offset == param)
          {
            sprintf(stack[++stack_pos], "\"%s\"", data->consts[k].value);
            break;
          }
        }
      }

      else if (op == PushI)
        sprintf(stack[++stack_pos], "%i", param);
      else if (op == PushF)
        sprintf(stack[++stack_pos], "%f", (float)param);
      else if (op == LoadI)
        sprintf(stack[++stack_pos], "%s", data->variables[param].name);
      else if (op == LoadF)
        sprintf(stack[++stack_pos], "%s", data->variables[param].name);
      else if (op == LoadS)
        sprintf(stack[++stack_pos], "\"%s\"", data->variables[param].name);
      else if (op == Not)
        shp_operator1(stack, &stack_pos, "!");
      else if (op == IsEqualI)
        shp_operator2(stack, &stack_pos, "==");
      else if (op == NotEqualI)
        shp_operator2(stack, &stack_pos, "!=");
      else if (op == IsGreaterI)
        shp_operator2(stack, &stack_pos, ">");
      else if (op == IsGreaterF)
        shp_operator2(stack, &stack_pos, ">");
      else if (op == IsLessI)
        shp_operator2(stack, &stack_pos, "<");
      else if (op == IsLessF)
        shp_operator2(stack, &stack_pos, "<");
      else if (op == IsGreaterEqualI)
        shp_operator2(stack, &stack_pos, ">=");
      else if (op == IsGreaterEqualF)
        shp_operator2(stack, &stack_pos, ">=");
      else if (op == IsLessEqualI)
        shp_operator2(stack, &stack_pos, "<=");
      else if (op == IsLessEqualF)
        shp_operator2(stack, &stack_pos, "<=");
      else if (op == And)
        shp_operator2(stack, &stack_pos, "&&");
      else if (op == Or)
        shp_operator2(stack, &stack_pos, "||");
      else if (op == AddI)
        shp_operator2(stack, &stack_pos, "+");
      else if (op == Pop)
        stack_pos--;
    }

    while (if_stack_pos-- > 0)
    {
      tab_offset--;
      shp_print("}\n", NULL);
    }

    tab_offset--;
    shp_print("}\n", NULL);
  }
  tab_offset--;
  shp_print("}\n", NULL);
  fclose(f);
}

// Main

void extract(brn_data* brn, char* filename, char* prefix)
{
  char filename2[256];
  if (prefix) {
    sprintf(filename2, "%s/%s", prefix, filename);
  } else {
    sprintf(filename2, "%s", filename);
  }
  if (strnstr(filename, ".BMP", 40) != 0 && access(filename2, F_OK ) != -1)
  {
    return;
  }

  printf("Extracting %s\n", filename2);
  if (strnstr(filename, ".BMP", 40))
  {
    bmp_data* bmp = brn_extract(brn, filename, (handler)bmp_handler);
    bmp_write(bmp, filename, prefix, 1, 0, 0);
    bmp_close(bmp);
  }
  else if (strnstr(filename, ".MUL", 40))
  {
    mul_data* mul = brn_extract(brn, filename, (handler)mul_handler);

    mkdir(filename, S_IRWXU);
    for (unsigned int i = 0; i < mul->count; i++)
    {
      char mul_filename[32];
      sprintf(mul_filename, "%u.BMP", i);
      bmp_write(&mul->maps[i], mul_filename, filename, 0, 0, 0);
    }

    mul_close(mul);
  }
  else if (strnstr(filename, ".BSP", 40))
  {
    bsp_data* bsp = brn_extract(brn, filename, (handler)bsp_handler);

    bsp_write(bsp, filename, NULL);

    for (unsigned int i = 0; i < bsp->surface_count; i++)
    {
      extract(brn, bsp->surfaces[i].texture_name, filename);
    }
    bsp_close(bsp);
  }
  else if (strnstr(filename, ".MOD", 40))
  {
    mkdir(filename, S_IRWXU);

    mod_data* mod = brn_extract(brn, filename, (handler)mod_handler);
    mod_write(mod, filename, filename);

    for (unsigned int i = 0; i < mod->mesh_count; i++)
    {
      for (unsigned int j = 0; j < mod->meshes[i].section_count; j++)
      {
        if (mod->meshes[i].sections[j].texture_file[0] == 0)
          continue;

        extract(brn, mod->meshes[i].sections[j].texture_file, filename);
      }
    }

    mod_close(mod);
  }
  else if (strnstr(filename, ".ACT", 40))
  {
    mkdir(filename, S_IRWXU);

    act_data* act = brn_extract(brn, filename, (handler)act_handler);

    char mod_filename[255];
    sprintf(mod_filename, "%s.MOD", act->model_name);
    mod_data* mod = brn_extract(brn, mod_filename, (handler)mod_handler);

    mod_write_act(mod, act, filename);

    for (unsigned int i = 0; i < mod->mesh_count; i++)
    {
      for (unsigned int j = 0; j < mod->meshes[i].section_count; j++)
      {
        if (mod->meshes[i].sections[j].texture_file[0] == 0 || mod->meshes[i].sections[j].texture_file[0] == '.')
          continue;

        extract(brn, mod->meshes[i].sections[j].texture_file, filename);
      }
    }

    mod_close(mod);
    act_close(act);
  }
  else if (strnstr(filename, ".SHP", 40) && strnstr(filename, "GK3.SHP", 40) == 0)
  {
    char txt[64];
    sprintf(txt, "%s.TXT", filename);

    shp_data* shp = brn_extract(brn, filename, (handler)shp_handler);
    shp_write(shp, txt);
    shp_close(shp);
  }
  else if (strnstr(filename, ".SCN", 40))
  {
    brn_extract(brn, filename, 0);

    scn_data* scn = brn_extract(brn, filename, (handler)scn_handler);
    extract(brn, scn->bsp, NULL);
    scn_close(scn);
  }
  else if (strnstr(filename, ".CUR", 40))
  {
    brn_extract(brn, filename, 0);

    char bmp_file[64];
    strncpy(bmp_file, filename, 64);
    strncpy(bmp_file + strlen(bmp_file) - 4, ".BMP", 4);

    cur_data* cur = brn_extract(brn, filename, (handler)cur_handler);
    bmp_data* bmp = brn_extract(brn, bmp_file, (handler)bmp_handler);

    mkdir(bmp_file, S_IRWXU);
    for (unsigned int i = 0; i < cur->frame_count; i++)
    {
      char bmp_output[32];
      sprintf(bmp_output, "%u.BMP", i);
      bmp_write(bmp, bmp_output, bmp_file, 1, i * 40, (i + 1) * 40);
    }

    bmp_close(bmp);
    cur_close(cur);
  }
  else
  {
    brn_extract(brn, filename, 0);
  }
}

void extract_multi(brn_data* brn, int count, char** filenames)
{
  if (count == 0)
  {
    printf("Usage: gk3 --props RC1_A.BSP\n");
    return;
  }

  for (int i = 0; i < count; i++)
  {
    if (!strnstr(filenames[i], ".BSP", 40))
    {
      printf("Props mode only works for BPS files\n");
      return;
    }
  }

  bsp_data* bsps[count];
  for (int i = 0; i < count; i++)
  {
    bsps[i] = brn_extract(brn, filenames[i], (handler)bsp_handler);
  }

  bsp_data* new_data = bsp_merge(count, bsps);
  bsp_write(new_data, filenames[0], NULL);

  for (int i = 0; i < count; i++)
  {
    for (unsigned int j = 0; j < bsps[i]->surface_count; j++)
    {
      extract(brn, bsps[i]->surfaces[j].texture_name, filenames[0]);
    }
  }

  for (int i = 0; i < count; i++)
  {
    bsp_close(bsps[i]);
  }
}

void extract_props(brn_data* brn, int count, char** filenames)
{
  if (count == 0)
  {
    printf("Usage: gk3 --multi RC1_A.BSP RC2.BSP\n");
    return;
  }

  for (int i = 0; i < count; i++)
  {
    if (!strnstr(filenames[i], ".BSP", 40))
    {
      printf("Multi mode only works for BPS files\n");
      return;
    }
  }

  for (int i = 0; i < count; i++)
  {
    bsp_data* bsp = brn_extract(brn, filenames[i], (handler)bsp_handler);

    for (unsigned int j = 0; j < bsp->model_count; j++)
    {
      bsp_write(bsp, filenames[i], bsp->models[j].name);
    }

    for (unsigned int j = 0; j < bsp->surface_count; j++)
    {
      extract(brn, bsp->surfaces[j].texture_name, filenames[i]);
    }

    bsp_close(bsp);
  }
}

int main(int argc, char** argv)
{
  brn_data* brn = brn_open((char*)"CORE.BRN", 1);

  if (argc == 1)
  {
    for(unsigned int i = 0; i < brn->count; i++)
    {
      printf("%s\n", brn->files[i].name);
    }
  }
  else if (strncmp(argv[1], "--multi", 7) == 0)
  {
    extract_multi(brn, argc - 2, &argv[2]);
  }
  else if (strncmp(argv[1], "--props", 7) == 0)
  {
    extract_props(brn, argc - 2, &argv[2]);
  }
  else if (strncmp(argv[1], "--find", 6) == 0)
  {
    for (unsigned int i = 0; i < brn->count; i++)
    {
      if (strstr(brn->files[i].name, argv[2]) != 0)
      {
        extract(brn, brn->files[i].name, NULL);
      }
    }
  }
  else
  {
    for (int i = 1; i < argc; i++)
    {
      extract(brn, argv[i], NULL);
    }
  }
  brn_close(brn);

  return 0;
}
