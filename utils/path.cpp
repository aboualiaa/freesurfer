/*
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "error.h"
#include "getline.h"

#include "path.h"

int PathReadMany(char *fname, int *num_read, PATH ***returned_paths) {
  FILE * fp;
  int    num_scanf;
  int    line_number;
  int    path_pno;
  int    num_points;
  char * line      = nullptr;
  size_t line_size = 1024;
  int    version;
  PATH * path      = nullptr;
  PATH **paths     = nullptr;
  int    num_paths = 0;
  float  x, y, z;
  int    vno;

  /* Try opening the file. */
  fp = fopen(fname, "r");
  if (nullptr == fp) {
    ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM, "Couldn't open %s\n", fname));
  }
  line_number = 0;

  line = (char *)malloc(line_size);

  /* Look for keywords... */
  while (!feof(fp)) {
    if (getline(&line, &line_size, fp) == -1) {
      ErrorPrintf(ERROR_BAD_FILE, "Couldn't read file\n");
    }
    line_number++;

    /* Skip comments. */
    if (line[0] == '#')
      continue;

    /* If this is the end of file, go to the end. */
    if (feof(fp))
      continue;

    /* VERSION keyword */
    if (0 == strncmp(line, "VERSION", 7)) {
      /* See if we recognize this version number. */
      num_scanf = sscanf(line, "VERSION %d", &version);
      if (1 != num_scanf) {
        fclose(fp);
        free(line);
        if (paths)
          free(paths);
        if (path)
          free(path);
        ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM,
                                    "Error reading file %s\n"
                                    "     line number %d\n"
                                    "     couldn't read version number\n",
                                    fname, line_number));
      }
      if (1 != version && 2 != version) {
        fclose(fp);
        free(line);
        if (paths)
          free(paths);
        if (path)
          free(path);
        ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM,
                                    "Error reading file %s\n"
                                    "     wrong version %d\n",
                                    fname, version));
      }
    } else if (0 == strncmp(line, "BEGINPATH", 9)) {
      /* Start a new path decsription. */
      if (getline(&line, &line_size, fp) == -1) {
        ErrorPrintf(ERROR_BAD_FILE, "Couldn't read file\n");
      }
      line_number++;
      if (0 != strncmp(line, "NUMVERTICES", 11) &&
          0 != strncmp(line, "NUMPOINTS", 9)) {
        fclose(fp);
        free(line);
        if (paths)
          free(paths);
        if (path)
          free(path);
        ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM,
                                    "Error reading file %s\n"
                                    "     line number %d\n"
                                    "     expected NUMVERTICES\n",
                                    fname, line_number));
      }

      /* Scan for the number of vertices. */
      num_scanf = sscanf(line, "NUMVERTICES %d", &num_points);
      if (1 != num_scanf)
        num_scanf = sscanf(line, "NUMPOINTS %d", &num_points);
      if (1 != num_scanf || feof(fp)) {
        fclose(fp);
        free(line);
        if (paths)
          free(paths);
        if (path)
          free(path);
        ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM,
                                    "Error reading file %s\n"
                                    "     line number %d\n"
                                    "     couldn't read NUMPOINTS number\n",
                                    fname, line_number));
      }

      /* Allocate our path object. */
      path = PathAlloc(num_points, fname);
      if (nullptr == path) {
        fclose(fp);
        free(line);
        if (paths)
          free(paths);
        if (path)
          free(path);
        ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM,
                                    "Error creating path of\n"
                                    "     size %d\n"
                                    "     line number %d\n"
                                    "     couldn't read NUMPOINTS number\n",
                                    num_points, fname, line_number));
      }

      /* Read in a line of coordinates for every point we
         have. */
      for (path_pno = 0; path_pno < num_points; path_pno++) {
        if (getline(&line, &line_size, fp) == -1) {
          ErrorPrintf(ERROR_BAD_FILE, "Couldn't read file\n");
        }
        line_number++;

        switch (version) {
        case 1:

          num_scanf = sscanf(line, "%f %f %f", &x, &y, &z);
          if (3 != num_scanf || feof(fp)) {
            fclose(fp);
            free(line);
            free(path);
            if (paths)
              free(paths);
            if (path)
              free(path);
            ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM,
                                        "Error reading file %s\n"
                                        "     line number %d\n"
                                        "     couldn't read three floats\n",
                                        fname, line_number));
          }
          vno = -1;
          break;
        case 2:
          num_scanf = sscanf(line, "%f %f %f %d", &x, &y, &z, &vno);
          if (4 != num_scanf || feof(fp)) {
            fclose(fp);
            free(line);
            free(path);
            if (paths)
              free(paths);
            if (path)
              free(path);
            ErrorReturn(ERROR_BADPARM,
                        (ERROR_BADPARM,
                         "Error reading file %s\n"
                         "     line number %d\n"
                         "     couldn't read three floats and an int\n",
                         fname, line_number));
          }
          break;
        }

        /* Add this coordinate to our label. */
        path->points[path_pno].x   = x;
        path->points[path_pno].y   = y;
        path->points[path_pno].z   = z;
        path->points[path_pno].vno = vno;
      }

      /* Make sure we got the ENDPATH keyword. */
      if (getline(&line, &line_size, fp) == -1) {
        ErrorPrintf(ERROR_BAD_FILE, "Couldn't read file\n");
      }
      line_number++;
      if (0 != strncmp(line, "ENDPATH", 7)) {
        fclose(fp);
        free(line);
        if (paths)
          free(paths);
        if (path)
          free(path);
        ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM,
                                    "Error reading file %s\n"
                                    "     line number %d\n"
                                    "     expected ENDPATH\n",
                                    fname, line_number));
      }

      /* Add the path to our array. */
      if (nullptr == paths) {
        paths = (PATH **)calloc(num_paths + 1, sizeof(PATH *));
      } else {
        paths = (PATH **)realloc(paths, (num_paths + 1) * sizeof(PATH *));
      }
      paths[num_paths] = path;
      num_paths++;
      path = nullptr;
    } else {
      /* Didn't get a keyword we're looking for. */
      ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM,
                                  "Error reading file %s\n"
                                  "     line number %d\n"
                                  "     no expected keyword found\n",
                                  fname, line_number));
    }
  }

  free(line);
  fclose(fp);

  *num_read       = num_paths;
  *returned_paths = paths;

  return (ERROR_NONE);
}

int PathWriteMany(char *fname, int num_paths, PATH **paths) {
  FILE *fp;
  int   path;
  int   path_pno;

  /* Try to open the file. */
  fp = fopen(fname, "w");
  if (nullptr == fp) {
    ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM, "Couldn't open %s\n", fname));
  }

  /* Write some header info. */
  fprintf(fp, "# Path file\n");

  /* Version keyword. */
  fprintf(fp, "VERSION 2\n");

  /* For each path... */
  for (path = 0; path < num_paths; path++) {
    /* Add BEGINPATH and NUMVERTICES keywords and info. */
    fprintf(fp, "BEGINPATH\n");
    fprintf(fp, "NUMPOINTS %d\n", paths[path]->n_points);

    /* For each vertex, write a line with the coordinate on it. */
    for (path_pno = 0; path_pno < paths[path]->n_points; path_pno++) {
      fprintf(fp, "%f %f %f %d\n", paths[path]->points[path_pno].x,
              paths[path]->points[path_pno].y, paths[path]->points[path_pno].z,
              paths[path]->points[path_pno].vno);
    }

    /* ENDPATH keyword. */
    fprintf(fp, "ENDPATH\n");
  }

  fclose(fp);

  return (ERROR_NONE);
}

PATH *PathAlloc(int n_points, const char *name) {
  PATH *path;

  if (n_points < 0)
    return nullptr;

  /* Allocate path struct. */
  path = (PATH *)malloc(sizeof(PATH));
  if (nullptr == path) {
    printf("ERROR: Couldn't allocate path.\n");
    return nullptr;
  }

  /* Set the number of points. */
  path->n_points = n_points;

  /* Copy in a name. */
  if (nullptr != name)
    strncpy(path->name, name, 100);
  else
    strcpy(path->name, "");

  /* Allocate the point storage. */
  path->points = (PATH_POINT *)calloc(n_points, sizeof(PATH_POINT));
  if (nullptr == path) {
    printf("ERROR: Couldn't allocate %d points in path.\n", n_points);
    free(path);
    return nullptr;
  }

  return path;
}

int PathFree(PATH **path) {
  if (nullptr == path || nullptr == (*path))
    ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM, "No path supplied."));

  /* Free the points array, then the path struct. */
  free((*path)->points);
  free((*path));
  *path = nullptr;

  return (ERROR_NONE);
}

int PathIsPathFile(char *fname) {
  FILE *fp           = nullptr;
  int   is_path_file = 0;

  /* Open the file. */
  fp = fopen(fname, "r");
  if (nullptr == fp)
    return 0;

  /* Run the stream version. */
  is_path_file = PathIsPathFileStream(fp);

  /* Close the file. */
  fclose(fp);

  return is_path_file;
}

int PathIsPathFileStream(FILE *fp) {
  char * line   = nullptr;
  size_t size   = 1024;
  char * needle = nullptr;
  int    found  = 0;

  found = 0;

  /* Line buffer. */
  line = (char *)malloc(size);

  while (!feof(fp) && !found) {
    /* Get a line. */
    if (getline(&line, &size, fp) == -1) {
      ErrorPrintf(ERROR_BAD_FILE, "Couldn't read file\n");
    }

    /* If it's a comment line. */
    if (line[0] == '#') {
      /* Look for the Path string. It's a path file if so. */
      needle = strstr(line, "Path");
      if (nullptr != needle) {
        found = 1;
        break;
      }
    }
  }

  free(line);

  return found;
}

int PathConvertToLabel(PATH *path, LABEL **label) {
  LABEL *new_label = nullptr;
  int    pno       = 0;

  if (nullptr == path)
    ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM, "Path pointer was null"));

  if (nullptr == label)
    ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM, "Label pointer not null"));

  /* Make a label the size of first path. */
  new_label = LabelAlloc(path->n_points, nullptr, nullptr);
  if (nullptr == new_label)
    ErrorReturn(ERROR_NO_MEMORY,
                (ERROR_NO_MEMORY, "Couldn't allocate label of %d points",
                 path->n_points));

  new_label->n_points = path->n_points;

  /* Write all the path points to the label. */
  for (pno = 0; pno < path->n_points; pno++) {
    new_label->lv[pno].x   = path->points[pno].x;
    new_label->lv[pno].y   = path->points[pno].y;
    new_label->lv[pno].z   = path->points[pno].z;
    new_label->lv[pno].vno = path->points[pno].vno;
  }

  /* Return the label. */
  *label = new_label;

  return (ERROR_NONE);
}

int PathCreateFromLabel(LABEL *label, PATH **path) {
  PATH *new_path = nullptr;
  int   pno      = 0;

  if (nullptr == label)
    ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM, "Label pointer was null"));

  if (nullptr == path)
    ErrorReturn(ERROR_BADPARM, (ERROR_BADPARM, "Path pointer was null"));

  /* Make the path. */
  new_path = PathAlloc(label->n_points, nullptr);
  if (nullptr == path)
    ErrorReturn(ERROR_NO_MEMORY,
                (ERROR_NO_MEMORY, "Couldn't allocate path of %d points",
                 label->n_points));

  /* Read points into the path from the label. */
  for (pno = 0; pno < label->n_points; pno++) {
    new_path->points[pno].x   = label->lv[pno].x;
    new_path->points[pno].y   = label->lv[pno].y;
    new_path->points[pno].z   = label->lv[pno].z;
    new_path->points[pno].vno = label->lv[pno].vno;
  }

  /* Return the path. */
  *path = new_path;

  return (ERROR_NONE);
}
