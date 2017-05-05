#include <libgimp/gimp.h>

static void query (void);
static void run   (const gchar      *name,
                   gint              nparams,
                   const GimpParam  *param,
                   gint             *nreturn_vals,
                   GimpParam       **return_vals);

GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,
  NULL,
  query,
  run
};

MAIN()

static void
query (void)
{
  static GimpParamDef args[] =
  {
    {
      GIMP_PDB_INT32,
      "run-mode",
      "Run mode"
    },
    {
      GIMP_PDB_IMAGE,
      "image",
      "Input image"
    }
  };

  gimp_install_procedure (
    "get-layers-offset",
    "Displays layers offsets in a dialog",
    "Displays layers offsets in a dialog",
    "Maxim Moseychuk",
    "Copyright Maxim Moseychuk",
    "2017",
    "Get offsets",
    "RGB*, GRAY*, INDEXED*",
    GIMP_PLUGIN,
    G_N_ELEMENTS(args), 0,
    args, NULL);

  gimp_plugin_menu_register ("get-layers-offset",
                             "<Image>/Filters/Layers");
}

static GString* get_padding(guint padding) {
  GString *result = g_string_new(NULL);
  while (padding) {
    g_string_append_c(result, '\t');
    --padding;
  }
  return result;
}

static GString* unsupported_process(const guint deph) {
  GString *result = g_string_new(NULL);
  GString *padding = get_padding(deph);
  g_string_printf(result, "%sunsupported item", padding->str);
  g_string_free(padding, TRUE);
  return result;
}

static GString* drawable_process(const gint layer, const guint deph) {
  GString *result = g_string_new(NULL);
  GString *padding = get_padding(deph);
  gint x, y;
  gimp_drawable_offsets(layer, &x, &y);
  g_string_printf(result, "%s%s: %i %i", padding->str, gimp_drawable_get_name(layer), x, y);
  g_string_free(padding, TRUE);
  return result;
}

static GString* group_process(const gint group, const guint deph) {
  GString *result = g_string_new(NULL);
  GString *padding = get_padding(deph);
  g_string_printf(result, "%s%s:", padding->str, gimp_item_get_name(group));
  gint num_children;
  gint *childrens = gimp_item_get_children(group, &num_children);

  for(gint i = 0; i < num_children; ++i) {
    GString *r = NULL;
    const gint item = childrens[i];
    if(gimp_item_is_group(item)) {
      r = group_process(item, deph + 1);
    } else if(gimp_item_is_drawable(item)) {
      r = drawable_process(item, deph + 1);
    } else {
      r = unsupported_process(deph + 1);
    }
    g_string_append(g_string_append_c(result, '\n'), r->str);
    g_string_free(r, TRUE);
  }

  g_string_free(padding, TRUE);
  return result;
}

static GString* image_process(const gint image) {
  GString *result = g_string_append_c(g_string_new(gimp_image_get_name(image)), ':');
  const guint deph = 0;
  gint layers_num;
  const gint *layers = gimp_image_get_layers(image, &layers_num);

  for(gint i = 0; i < layers_num; ++i) {
    GString *r = NULL;
    const gint item = layers[i];
    if(gimp_item_is_group(item)) {
      r = group_process(item, deph + 1);
    } else if(gimp_item_is_drawable(item)) {
      r = drawable_process(item, deph + 1);
    } else {
      r = unsupported_process(deph + 1);
    }
    g_string_append(g_string_append_c(result, '\n'), r->str);
    g_string_free(r, TRUE);
  }

  return result;
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  // static GimpParam  values[1];
  // GimpPDBStatusType status = GIMP_PDB_SUCCESS;
  GimpRunMode       run_mode;

  /* Setting mandatory output values */
  // *nreturn_vals = 1;
  // *return_vals  = values;

  // values[0].type = GIMP_PDB_STATUS;
  // values[0].data.d_status = status;

  const gint32 image = param[1].data.d_image;
  GString *msg = image_process(image);
  run_mode = param[0].data.d_int32;

  if (run_mode != GIMP_RUN_NONINTERACTIVE)
    g_message(msg->str);

  g_string_free(msg, TRUE);
}
