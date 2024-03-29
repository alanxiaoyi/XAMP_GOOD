
#include <gtk/gtk.h>
#include "interface.h"


using namespace std;
typedef struct{
GtkWidget *text;
GtkWidget *iotext;
GtkWidget *desctext;
GtkWidget	*editor;
}textstruct;

GtkWidget *main_window, *result_text; 
int combo_count=0;
list<model_class>::iterator themodel;
GThread *thread;

/*processing_dialog
*/

void close_processing_dialog( GtkWidget *widget){
	g_print("You stop the process\n"); 
	exit(0);
}



static gpointer thread_func(gpointer Gpointer){

	gdk_threads_enter();

	GtkWidget *processing_dialog = gtk_message_dialog_new (GTK_WINDOW(main_window),
								 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 GTK_MESSAGE_QUESTION,
                                 GTK_BUTTONS_CLOSE,
                                 "Processing...model is processing\nClick cancel if no response");	
	g_signal_connect(processing_dialog,"response", G_CALLBACK(close_processing_dialog),NULL);
	gtk_widget_show_all (processing_dialog);	
	gdk_threads_leave();	
	string result="";
	char tmp[256];
	char cmd_char[256];
	string cmd;
	g_print("push enter_button\n"); 
	strcpy(cmd_char,gtk_entry_get_text(GTK_ENTRY(Gpointer)));
	cmd=string(cmd_char);
	init_model(themodel , 0, cmd );	
	ifstream infile("pipe.tmp");	
	if(!infile) {cout<<"open pipe.tmp fail(no output to stdio)"<<endl; return NULL;}	
	while(infile.getline(tmp,256)){
		result=result+"\n"+string(tmp)	;
	}	
	gdk_threads_enter();
	gtk_label_set_text(GTK_LABEL(result_text),result.c_str());	
	gtk_widget_destroy(processing_dialog);
	gdk_threads_leave();

	remove("pipe.tmp"); 						//delete the tmp file;

	g_thread_exit(NULL);
	return NULL;
}




/*create icon
*/
GdkPixbuf *create(const gchar * filename)
{
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%sn", error->message);
      g_error_free(error);
   }
   return pixbuf;
}


/*Exit button
*/
void cb_qbutton(GtkWidget *widget, gpointer data) {
	g_print("push exit_button\n"); 
	gtk_main_quit(); 
} 

/*enter button
*/
void cb_ebutton(GtkWidget *widget,GtkEntry *editor) {
	
//try to fork new process to run the model since we don't want it block the window
			
//	signal(SIGCHLD, handler);
	thread=NULL;
	GError* error=NULL;
	thread = g_thread_create( thread_func, (gpointer)editor,
                              FALSE, &error );

	if (!thread){
		cout<<"call model as child thread fail!"<<endl;
	}
} 

/*Error dialog
*/
void error_loading_dialog(){
	GtkWidget *error_dialog = gtk_message_dialog_new (GTK_WINDOW(main_window),
								 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_CLOSE,
                                 "ERROR loading XML file");	
	gtk_widget_show(error_dialog);
	g_signal_connect_swapped (error_dialog, "response",
								G_CALLBACK (gtk_widget_destroy),
								error_dialog);								 
}	

/*parse input button
*/
void cb_ibutton(GtkWidget *widget, gpointer data) {
				g_print("push parse input button\n"); 
				parse_input_file(input_file_name);
} 

/* input deduction button
*/
void cb_dbutton(GtkWidget *widget, gpointer data) {
				g_print("push input deduction button\n"); 
				input_deduction();
} 


/*parse config file button
*/
void cb_cbutton(GtkWidget *widget, GtkWidget * combo) {
				list<model_class>::iterator itm;
				g_print("push config button\n"); 
				GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(main_window),
                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 GTK_MESSAGE_QUESTION,
                                 GTK_BUTTONS_YES_NO,
                                 "Do you want to create a new input file?");
				gint result = gtk_dialog_run (GTK_DIALOG (dialog));				//get the user's choice
				gtk_widget_destroy (dialog);

				if(result==-8) {
					if(!parse_config(config_file_name,true/*create new input file or not*/))
						error_loading_dialog();					 
					
				}
				else {
					if(!parse_config(config_file_name,false/*create new input file or not*/))	
						error_loading_dialog()	;					
				}
			if(combo_count!=0){
				for(int i=0; i<combo_count; i++)					
					gtk_combo_box_remove_text (GTK_COMBO_BOX(combo), 0);
			}
			combo_count=0;
			for(itm=model_list.begin(); itm!=model_list.end();itm++){						//init combo box
				combo_count++;
				stringstream ss;
				ss<<itm->num;
				gtk_combo_box_append_text(GTK_COMBO_BOX(combo), ((ss.str())+". "+(itm->name)).c_str());
			}
} 

/*combo menu choose
*/
void cb_menu(GtkComboBox *combo, textstruct* data){
	list<model_class>::iterator itm;

	GtkWidget *otherwgt1=(GtkWidget*) (data->text);
	GtkWidget *otherwgt2=(GtkWidget*) (data->iotext);
	GtkWidget *otherwgt3=(GtkWidget*) (data->desctext);
	GtkWidget *otherwgt4=(GtkWidget*) (data->editor);
	int a=gtk_combo_box_get_active (combo);
	string tmp;
	for(itm=model_list.begin(); itm!=model_list.end();itm++){
		if(itm->num==a+1) break;
	}
	if(itm!=model_list.end()){
		themodel=itm;
		int n=0;
		tmp="input: \n\n";
		while(itm->input[n][0]!=""){
			tmp=tmp+itm->input[n][0]+": "+itm->input[n][1]+"\t\t//"+itm->input[n][2]+"\n";
			n++;			
		}
		gtk_entry_set_text (GTK_ENTRY(otherwgt4), (itm->dft[0]).c_str());
		gtk_label_set_text(GTK_LABEL(otherwgt2),tmp.c_str());
		gtk_label_set_text(GTK_LABEL(otherwgt1), ("User Guide for The Model:\n\n"+itm->guide).c_str());
		gtk_label_set_text(GTK_LABEL(otherwgt3), ("Description for The Model:\n\n"+itm->comment).c_str());

	}
}

int call_gui() { 
	g_thread_init(NULL);
    gdk_threads_init();
	gtk_init(NULL, NULL);
	GtkWidget *hbox_head,*hbox1, *hbox2, *hbox3 ,*hbox4,*hbox4_1,*hbox4_2,*hbox_desc, *vbox;
	GtkWidget *exit_button,*input_button,*deduction_button,*enter_button,*config_button,  *combo,*combotext,*headtext;
	GtkWidget *buttonbox1,*frame0, *frame1, *frame2, *frame3, *frame4, *frame5, *vbox_s3,*vbox_s2;
	GtkWidget *scrolledwindow1,*scrolledwindow2,*scrolledwindow3,*scrolledwindow4,*scrolledwindow_all;
	GtkObject *vadjust,*hadjust;
	textstruct *textmove=g_slice_new(textstruct);

		 
//	gtk_init(NULL, NULL);
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL); 
	gtk_window_set_title(GTK_WINDOW(main_window), "Model Platform"); 
	gtk_window_set_default_size(GTK_WINDOW(main_window), 930,830); 
	gtk_window_set_policy (GTK_WINDOW(main_window), true, true, true);
	gtk_window_set_icon(GTK_WINDOW(main_window), create("icon.png"));

	/*combo box
	*/
	combo=gtk_combo_box_new_text ();
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX(combo),true);
	gtk_combo_box_set_title (GTK_COMBO_BOX(combo),"choose a model");

	/*label
	*/
	textmove->text=gtk_label_new("");
	textmove->iotext=gtk_label_new("");
	textmove->desctext=gtk_label_new("");
	result_text=gtk_label_new("");
	gtk_label_set_selectable (GTK_LABEL(result_text), true);
	headtext=gtk_label_new("This is the GUI model platform. Please use it as 3 steps as below, and also refer to README for more information. Copyright 2012 by Yipeng. All Rights Reserved.");
	gtk_widget_set_size_request(  headtext , 700, -1 );
	gtk_label_set_line_wrap (GTK_LABEL(headtext), true);	
	combotext=gtk_label_new("Choose model:");
	gtk_label_set_line_wrap( GTK_LABEL( result_text), TRUE );
	gtk_widget_set_size_request(  result_text , 700, -1 );
	gtk_label_set_line_wrap (GTK_LABEL(textmove->text), true);			//guide text wrap
	gtk_widget_set_size_request(textmove->text, 370,-1);				//wrap on the bound of the scrolled window
	gtk_label_set_line_wrap (GTK_LABEL(textmove->desctext), true);		//description text wrap
	gtk_widget_set_size_request(textmove->desctext, 800,-1);				//wrap on the bound of the scrolled window


	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(cb_menu),textmove);
	
	/*scroll window
	*/
	scrolledwindow1 = gtk_scrolled_window_new(NULL,NULL);
	scrolledwindow2 = gtk_scrolled_window_new(NULL,NULL);
	scrolledwindow3 = gtk_scrolled_window_new(NULL,NULL);
	scrolledwindow4 = gtk_scrolled_window_new(NULL,NULL);
	scrolledwindow_all = gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scrolledwindow1, 430, 170);		//guide
	gtk_widget_set_size_request(scrolledwindow2, 430, 170);		//iotext
	gtk_widget_set_size_request(scrolledwindow3,800, 170);		//desctext
	gtk_widget_set_size_request(scrolledwindow4,700, 150);		//resulttext

	
	/*editable blank
	*/
	textmove->editor = gtk_entry_new(); 
	gtk_entry_set_text(GTK_ENTRY(textmove->editor),"Enter command for specified model HERE");
	

	/*button variable
	*/
	exit_button = gtk_button_new_with_label("Exit");
	input_button = gtk_button_new_with_label("Parse Input File");
	deduction_button = gtk_button_new_with_label("Input Deduction");
	config_button = gtk_button_new_with_label("Parse config and create new input file");
	enter_button = gtk_button_new_with_label("Enter");
	g_signal_connect(G_OBJECT(exit_button), "clicked", G_CALLBACK(cb_qbutton),NULL);
	g_signal_connect(G_OBJECT(input_button), "clicked", G_CALLBACK(cb_ibutton),NULL);
	g_signal_connect(G_OBJECT(deduction_button), "clicked", G_CALLBACK(cb_dbutton),NULL);
	g_signal_connect(G_OBJECT(enter_button), "clicked", G_CALLBACK(cb_ebutton),GTK_ENTRY(textmove->editor));
	g_signal_connect(G_OBJECT(config_button), "clicked", G_CALLBACK(cb_cbutton),combo);

	/*box
	*/
	vbox=gtk_vbox_new(false, 10);
	vbox_s3=gtk_vbox_new(false, 10);
	vbox_s2=gtk_vbox_new(false, 10);
	hbox_head = gtk_hbox_new(FALSE, 10);
	hbox1 = gtk_hbox_new(FALSE, 10);
	hbox2 = gtk_hbox_new(FALSE, 10);
	hbox3 = gtk_hbox_new(FALSE, 10);
	hbox4_2 = gtk_hbox_new(FALSE, 10);				//guid text box
	hbox4_1 = gtk_hbox_new(FALSE, 10);				//input text box
	hbox4 = gtk_hbox_new(FALSE, 10);				// box of the two above
	hbox_desc=gtk_hbox_new(FALSE,10);
	/*assmeble box
	*/
	 gtk_box_pack_start(GTK_BOX(hbox_head), headtext, TRUE, TRUE, 10);
	 gtk_box_pack_start(GTK_BOX(hbox1), textmove->editor, TRUE, TRUE, 10);
	 gtk_box_pack_start(GTK_BOX(hbox3), exit_button, false, false, 10);
	 gtk_box_pack_start(GTK_BOX(hbox3), enter_button, false, false, 10);
	 gtk_box_pack_start(GTK_BOX(hbox2), combotext, false, false, 3);
	 gtk_box_pack_start(GTK_BOX(hbox2), combo, false, false, 10);
	 gtk_box_pack_start(GTK_BOX(hbox_desc), textmove->desctext, false, false, 10);
	 gtk_box_pack_start(GTK_BOX(hbox4_2), textmove->text, false, false, 10);
	 gtk_box_pack_start(GTK_BOX(hbox4_1), textmove->iotext, false, false, 10);	 
	 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledwindow1),hbox4_2);
	 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledwindow2),hbox4_1);
	 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledwindow3),hbox_desc);
	 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledwindow4),result_text);
	 gtk_box_pack_start(GTK_BOX(vbox_s2), hbox2, false, false, 3);
	 gtk_box_pack_start(GTK_BOX(vbox_s2), scrolledwindow3, false, false, 3);
	 gtk_box_pack_start(GTK_BOX(hbox4), scrolledwindow1, false, false, 5);
	 gtk_box_pack_start(GTK_BOX(hbox4), scrolledwindow2, false, false, 5);
	 gtk_box_pack_start(GTK_BOX(vbox_s3), hbox4, false, false, 3);
	 gtk_box_pack_start(GTK_BOX(vbox_s3), hbox1, false, false, 3);
	 gtk_box_pack_start(GTK_BOX(vbox_s3), hbox3, false, false, 3);



	 /*button box
	 */
	buttonbox1=gtk_hbutton_box_new ();
	gtk_container_add(GTK_CONTAINER(buttonbox1), config_button); 	 
	gtk_container_add(GTK_CONTAINER(buttonbox1), input_button); 
	gtk_container_add(GTK_CONTAINER(buttonbox1), deduction_button);
	gtk_button_box_set_layout (GTK_BUTTON_BOX(buttonbox1),GTK_BUTTONBOX_SPREAD);
	gtk_box_set_spacing (GTK_BOX(buttonbox1),5);
	
	/*frame
	*/
	frame1=gtk_frame_new ("Step1. Parse configure/input file");
	frame2=gtk_frame_new ("Step2. Choose a model and read the description");	
	frame3=gtk_frame_new ("Step3. Enter command as instruction");	
	frame4=gtk_frame_new ("Result");	
	gtk_container_add(GTK_CONTAINER(frame1), buttonbox1); 	
	gtk_container_add(GTK_CONTAINER(frame2), vbox_s2); 
	gtk_container_add(GTK_CONTAINER(frame3), vbox_s3); 	 
	gtk_container_add(GTK_CONTAINER(frame4), scrolledwindow4); 
	
		
			
	/*assmeble frame
	*/
	 gtk_box_pack_start(GTK_BOX(vbox), hbox_head, false, false,5);	 
	 gtk_box_pack_start(GTK_BOX(vbox), frame1, false, false,5);	 
	 gtk_box_pack_start(GTK_BOX(vbox), frame2, false, false,5);
	 gtk_box_pack_start(GTK_BOX(vbox), frame3, false, false,5);
	 gtk_box_pack_start(GTK_BOX(vbox), frame4, false, false,5);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledwindow_all),vbox); 
	
	gtk_container_add(GTK_CONTAINER(main_window), scrolledwindow_all); 
	gtk_widget_show_all(main_window); 
	
    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();
	 return 0;
}