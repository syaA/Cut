
#include "stdafx.h"

#include "util.h"
#include "scene.h"
#include "figure.h"

#include "pmx_loader.h"
#include "resource_repository.h"

#include "font.h"
#include "gui.h"
#include "glfw_util.h"


namespace
{
void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    return;
  }

  gui::glfw_input_key(world()->get<gui::system::ptr_t>("gui"), window, key, scancode, action, mods);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mod)
{
  if (gui::glfw_mouse_button(world()->get<gui::system::ptr_t>("gui"), window, button, action, mod)) {
    return;
  }

  auto cc = world()->get<camera_control::ptr_t>("camera_control");
  cc->on_mouse_button(window, button, action, mod);
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
  gui::glfw_cursor_move(world()->get<gui::system::ptr_t>("gui"), window, x, y);

  auto cc = world()->get<camera_control::ptr_t>("camera_control");
  cc->on_cursor_move(window, x, y);
}

void cursor_enter_callback(GLFWwindow *window, int enter)
{
  gui::glfw_cursor_enter(world()->get<gui::system::ptr_t>("gui"), window, enter);

  auto cc = world()->get<camera_control::ptr_t>("camera_control");
  cc->on_cursor_enter(window, enter);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  if (gui::glfw_mouse_scroll(world()->get<gui::system::ptr_t>("gui"), window, xoffset, yoffset)) {
    return;
  }

  auto cc = world()->get<camera_control::ptr_t>("camera_control");
  cc->on_scroll(window, xoffset, yoffset);
}

entity_world s_world;

}	// end of anonymus namespace


entity_world *world() {
  return &s_world;
}


int main(int argc, char **argv)
{
  GLFWwindow* window;

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  window = glfwCreateWindow(960, 720, "Cut", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);
  glfwSetScrollCallback(window, scroll_callback);

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);

  std::cout << GLVersion.major << "." << GLVersion.minor << std::endl;

  
  world()->add("resource_repository", std::make_shared<resource_repository>());
  auto rm = world()->get<resource_repository::ptr_t>("resource_repository");
  {
    // ダミーテクスチャを作る.
    auto blacktex = texture::make();
    glBindTexture(GL_TEXTURE_2D, blacktex->texture_globj());
    uint8_t black[] = { 0x00 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, black);
    rm->add("tex_black", blacktex);
    auto whitetex = texture::make();
    glBindTexture(GL_TEXTURE_2D, whitetex->texture_globj());
    uint8_t white[] = { 0xff };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, white);
    rm->add("tex_white", whitetex);
  }

  auto pmx_shader = std::make_shared<shader>();
  assert(pmx_shader->compile_from_source_file("assets/shader/pmx.vsh", "assets/shader/pmx.fsh"));
  rm->add("pmx_shader", pmx_shader);

  std::string modelname;
  if (argc > 1) {
    modelname = argv[1];
  } else {
    std::ifstream f("assets/default_model.txt");
    if (f.is_open()) {
      f >> modelname;
    }
    modelname = "assets/" + modelname;
  }
  auto pmx_model = std::make_shared<model>();
  load_pmx(pmx_model.get(),
           modelname.c_str(),
           rm.get());
  rm->add("main_pmx", pmx_model);

  world()->add("main_scene", std::make_shared<scene>());
  auto scn = world()->get<scene::ptr_t>("main_scene");
  scn->root_camera() = 
    camera(vec3(0.f, 10.f, -30.f),
           vec3(0.f, 10.f, 0.f),
           vec3(0.f, 1.f, 0.f),
           deg2rad(45.f), 1.f, 0.1f, 100.f);

  scn->add_node(scene_node::make<model_node>(pmx_model, pmx_shader));

  world()->add("figure_manager", std::make_shared<figure::manager>());
  auto fm = world()->get<figure::manager::ptr_t>("figure_manager");
  fm->append_to(scn, figure::coordinator(matrix::identity() * 10.0f));

  world()->add("camera_control", std::make_shared<camera_control>(scn->root_camera()));
  auto cc = world()->get<camera_control::ptr_t>("camera_control");

  auto font_face = std::make_shared<font::face>("assets/ui/mplus-1m-regular.ttf", 0);
  auto font_shader = std::make_shared<shader>();
  assert(font_shader->compile_from_source_file("assets/shader/font.vsh", "assets/shader/font.fsh"));
  auto font_renderer = std::make_shared<font::renderer>(font_face, font_shader);
  world()->add("font_renderer", font_renderer);

  auto gui_shader = std::make_shared<shader>();
  assert(gui_shader->compile_from_source_file("assets/shader/gui.vsh", "assets/shader/gui.fsh"));
  auto gui_system = gui::system::create(gui_shader, font_renderer);
  world()->add("gui", gui_system);
  bool visible_mouse_point = false;
  int morph = 0;
  int mode = 0;
  {
    auto win = gui_system->add_window(u"てすとウィンドウ");
    win->add_child<gui::check_box>(u"マウス座標", &visible_mouse_point);
    win->add_child<gui::button>(u"ボタン", [=](){ std::cout << "click!" << std::endl; });
    auto grp = win->add_child<gui::group>(u"グループ", false);
    auto cmb = grp->add_child<gui::combo_box>(u"コンボボックス", &morph);
    cmb->add_item(u"一つ目のアイテム");
    cmb->add_item(u"2nd item");
    cmb->add_item(u"第三のもの");
    grp->add_child<gui::radio_button>(u"ラジオボタン０", &mode, 0)->set_layout_way(gui::LayoutWay_Horizon);
    grp->add_child<gui::radio_button>(u"ラジオボタン１", &mode, 1)->set_layout_way(gui::LayoutWay_Horizon);
    grp->add_child<gui::radio_button>(u"ラジオボタン２", &mode, 2);
    win->add_child<gui::label>(u"ラベル");
    win->add_child<gui::text_box>(u"カメラ", [=](){ return gui::to_s(scn->root_camera().eye(),
                                                                     std::showpos, std::fixed, std::showpoint, std::setprecision(2));});
  }
  {
    auto win = gui_system->add_window(u"test window");
    win->add_child<gui::check_box>(u"mouse position", &visible_mouse_point);
    win->add_child<gui::button>(u"button", [=](){ std::cout << "click!" << std::endl; });
    auto grp = win->add_child<gui::group>(u"group");
    auto cmb = grp->add_child<gui::combo_box>(u"combo_box", &morph);
    cmb->add_item(u"1st item");
    cmb->add_item(u"２つめのアイテム");
    cmb->add_item(u"3rd item");
    grp->add_child<gui::radio_button>(u"radio_button0", &mode, 0)->set_layout_way(gui::LayoutWay_Horizon);
    grp->add_child<gui::radio_button>(u"radio_button1", &mode, 1)->set_layout_way(gui::LayoutWay_Horizon);
    grp->add_child<gui::radio_button>(u"radio_button2", &mode, 2);
    win->add_child<gui::label>(u"label");
    win->add_child<gui::text_box>(u"camera", [=](){ return gui::to_s(scn->root_camera().eye(),
                                                                     std::showpos, std::fixed, std::showpoint, std::setprecision(2));});
  }
  {
    auto win = gui_system->add_window(u"gui param");
    win->add_child(gui::slider<float>::create(u"color0.r", &gui_system->property().frame_color0.r, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"color0.g", &gui_system->property().frame_color0.g, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"color0.b", &gui_system->property().frame_color0.b, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"color0.a", &gui_system->property().frame_color0.a, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"color1.r", &gui_system->property().frame_color1.r, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"color1.g", &gui_system->property().frame_color1.g, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"color1.b", &gui_system->property().frame_color1.b, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"color1.a", &gui_system->property().frame_color1.a, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"active0.r", &gui_system->property().active_color.r, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"active0.g", &gui_system->property().active_color.g, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"active0.b", &gui_system->property().active_color.b, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"active0.a", &gui_system->property().active_color.a, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"active1.r", &gui_system->property().semiactive_color.r, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"active1.g", &gui_system->property().semiactive_color.g, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"active1.b", &gui_system->property().semiactive_color.b, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"active1.a", &gui_system->property().semiactive_color.a, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"font.r", &gui_system->property().font_color.r, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"font.g", &gui_system->property().font_color.g, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"font.b", &gui_system->property().font_color.b, 0.f, 1.f));
    win->add_child(gui::slider<float>::create(u"font.a", &gui_system->property().font_color.a, 0.f, 1.f));

    win->add_child(gui::numeric_up_down<int>::create(
      u"font size", &gui_system->property().font_size, 8, 36, 1, [=](){ gui_system->recalc_layout(); }));
    win->add_child(gui::numeric_up_down<float>::create(
      u"round", &gui_system->property().round, 0.f, 20.f, 1.f, [=](){ gui_system->recalc_layout(); }));
    win->add_child(gui::numeric_up_down<float>::create(
      u"mergin", &gui_system->property().mergin, 0.f, 10.f, 1.f, [=](){ gui_system->recalc_layout(); }));
    win->add_child(gui::numeric_up_down<float>::create(
      u"tickness", &gui_system->property().tickness, 0.f, 10.f, 1.f, [=](){ gui_system->recalc_layout(); }));
  }
  gui_system->calc_layout();
  
  while (!glfwWindowShouldClose(window)) {

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    font_renderer->set_screen_size(width, height);
    gui_system->set_screen_size(width, height);

    gui_system->update();

    glfwPollEvents();

    float aspect = width / (float)height;

    glViewport(0, 0, width, height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glEnablei(GL_BLEND, 0);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    scn->root_camera().set_aspect(aspect);
    cc->apply_to(&scn->root_camera());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    scn->draw();

    gui_system->draw();

    if (visible_mouse_point) {
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      std::basic_stringstream<char16_t> ss;
      ss << x << ", " << y;
      font_renderer->render({(float)x, (float)y}, {16, 16}, {0.f, 0.f, 0.f, 1.f}, ss.str());
    }


    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  
  exit(EXIT_SUCCESS);
}
