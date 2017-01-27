#include "gk3.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// Output Format Structures

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

// Writers

void bmp_write(bmp_data* data, char* filename)
{
  if (data == NULL) return;

  FILE* f = fopen(filename, "w");

  // Write Header

  bmp_file_header h =
  {
    .magic = "BM",
    .size = data->height * (data->width * 3 + data->width % 4) + sizeof(bmp_file_header),
    .offset = sizeof(bmp_file_header),
    .header_size = sizeof(bmp_file_header) - 14,
    .width = data->width,
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

      char r = (char)((pixel & 0x001f) * 8);
      char g = (char)(((pixel & 0x07e0) >> 5) * 4);
      char b = (char)(((pixel & 0xf800) >> 11) * 8);

      fwrite(&r, sizeof(char), 1, f);
      fwrite(&g, sizeof(char), 1, f);
      fwrite(&b, sizeof(char), 1, f);
    }
  }

  // Close File

  fclose(f);
}

void mul_write(mul_data* data)
{
  for (unsigned int i = 0; i < data->count; i++)
  {
  }
}

void bsp_write(bsp_data* data, char* filename, char* matname)
{
  FILE* f = fopen(filename, "w");
  FILE* m = fopen(matname, "w");

  fprintf(f, "mtllib %s\n", matname);

  for (unsigned int i = 0; i < data->vertice_count; i++)
    fprintf(f, "v %f %f %f\n", data->vertices[i].x, data->vertices[i].y, data->vertices[i].z);

  for (unsigned int i = 0; i < data->vertice_count; i++)
    fprintf(f, "vt %f %f\n", data->coords[i].u, data->coords[i].v);

  for (unsigned int i = 0; i < data->vertice_count; i++)
    fprintf(f, "vn %f %f %f\n", data->normals[i].x, data->normals[i].y, data->normals[i].z);

  for (unsigned int i = 0; i < data->model_count; i++)
  {
    fprintf(f, "g group_%s\n", data->models[i].name);

    for (unsigned int j = 0; j < data->surface_count; j++)
    {
      if (data->surfaces[j].model_index != i)
        continue;

      fprintf(f, "usemtl group_%s_%i\n", data->models[i].name, j);
      fprintf(m, "newmtl group_%s_%i\n", data->models[i].name, j);
      fprintf(m, "map_Kd %s.BMP\n", data->surfaces[j].texture_name);

      for (unsigned int k = 0; k < data->indice_count; k++)
      {
        if (data->indices[k].surface_index != j)
          continue;

        fprintf(f, "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
          data->indices[k].a + 1, data->indices[k].a + 1, data->indices[k].a + 1,
          data->indices[k].b + 1, data->indices[k].b + 1, data->indices[k].b + 1,
          data->indices[k].c + 1, data->indices[k].c + 1, data->indices[k].c + 1);        
      }
    }
  }

  fclose(m);
  fclose(f);
}

void mod_write(mod_data* data, char* filename, char* matname)
{
  FILE* f = fopen(filename, "w");
  FILE* m = fopen(matname, "w");

  fprintf(f, "mtllib %s\n", matname);

  for (unsigned int i = 0; i < data->mesh_count; i++)
  {
    for (unsigned int j = 0; j < data->meshes[i].section_count; j++)
    {
      for (unsigned int k = 0; k < data->meshes[i].sections[j].vertice_count; k++)
      {
        vertice v = data->meshes[i].sections[j].vertices[k];
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
        fprintf(m, "map_Kd %s.BMP\n", data->meshes[i].sections[j].texture_file);

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

void extract(brn_data* brn, char* filename)
{
  if (strnstr(filename, ".BMP", 40))
  {
    bmp_data* bmp = brn_extract(brn, filename, (handler)bmp_handler);
    bmp_write(bmp, filename);
    bmp_close(bmp);
  }
  else if (strnstr(filename, ".MUL", 40))
  {
    mul_data* mul = brn_extract(brn, filename, (handler)mul_handler);
    mul_write(mul);
    mul_close(mul);
  }
  else if (strnstr(filename, ".BSP", 40))
  {
    char obj[64], mtl[64];
    sprintf(obj, "%s/%s.OBJ", filename, filename);
    sprintf(mtl, "%s/%s.MTL", filename, filename);

    bsp_data* bsp = brn_extract(brn, filename, (handler)bsp_handler);

    mkdir(filename, S_IRWXU);
    bsp_write(bsp, obj, mtl);

    for (unsigned int i = 0; i < bsp->surface_count; i++)
    {
      char bmpa[64], bmpf[64];
      sprintf(bmpa, "%s.BMP", bsp->surfaces[i].texture_name);
      for(int j = 0; (bmpa[j] = (char)toupper(bmpa[j])); j++);
      sprintf(bmpf, "%s/%s", filename, bmpa);

      bmp_data* bmp = brn_extract(brn, bmpa, (handler)bmp_handler);
      bmp_write(bmp, bmpf);
      bmp_close(bmp);
    }
    bsp_close(bsp);
  }
  else if (strnstr(filename, ".MOD", 40))
  {
    mkdir(filename, S_IRWXU);

    char obj[64], mtl[64];
    sprintf(obj, "%s/%s.OBJ", filename, filename);
    sprintf(mtl, "%s/%s.MTL", filename, filename);

    mod_data* mod = brn_extract(brn, filename, (handler)mod_handler);
    mod_write(mod, obj, mtl);

    for (unsigned int i = 0; i < mod->mesh_count; i++)
    {
      for (unsigned int j = 0; j < mod->meshes[i].section_count; j++)
      {
        if (mod->meshes[i].sections[j].texture_file[0] != 0)
        {
          char bmpa[64], bmpf[64];
          sprintf(bmpa, "%s.BMP", mod->meshes[i].sections[j].texture_file);
          sprintf(bmpf, "%s/%s", filename, bmpa);

          bmp_data* bmp = brn_extract(brn, bmpa, (handler)bmp_handler);
          bmp_write(bmp, bmpf);
          bmp_close(bmp);
        }
      }
    }

    mod_close(mod);
  }
  else if (strnstr(filename, ".ACT", 40))
  {
    act_data* act = brn_extract(brn, filename, (handler)act_handler);
    act_close(act);
  }
  else if (strnstr(filename, ".SHP", 40))
  {
    char txt[64];
    sprintf(txt, "%s.TXT", filename);

    shp_data* shp = brn_extract(brn, filename, (handler)shp_handler);
    shp_write(shp, txt);
    shp_close(shp);
  }
  else if (strnstr(filename, ".SCN", 40))
  {
    scn_data* scn = brn_extract(brn, filename, (handler)scn_handler);
    // extract(brn, scn->bsp);

    for (int i = 0; i < 6; i++)
    {
      if (scn->skybox[i][0] == 0) continue;

      char bmpa[64], bmpf[64];
      sprintf(bmpa, "%s", scn->skybox[i]);
      sprintf(bmpf, "%s/%s", scn->bsp, bmpa);
      
      // bmp_data* bmp = brn_extract(brn, bmpa, bmp_handler);
      // bmp_write(bmp, bmpf);
      // bmp_close(bmp);
    }
    scn_close(scn);
  }
  else if (strnstr(filename, ".SIF", 40) || strnstr(filename, ".STK", 40))
  {
    brn_extract(brn, filename, ini_handler);
  }
  else
  {
    brn_extract(brn, filename, 0);
  }
}

int main(int argc, char** argv)
{
  brn_data* brn = brn_open((char*)"CORE.BRN", 1);

  if (argc < 2)
  {
    for(unsigned int i = 0; i < brn->count; i++)
      printf("%s\n", brn->files[i].name);
  }
  else
  {
    extract(brn, argv[1]);    
  }
  brn_close(brn);

  return 0;
}
