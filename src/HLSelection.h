
typedef enum SH_PLACE{
	SH_INIT ,
	SH_UPDATE ,
	SH_MODIF 
};


void	HLS_init();
void	HLS_release();

/************************************************************************/
/* For highlight                                                                     */
/************************************************************************/
void	HLS_Update_selection( UINT place );


/************************************************************************/
/* for edit                                                                     */
/************************************************************************/
void	HLS_Edit_selection_start();
//void	HLS_Edit_selection( );
void	HLS_Edit_selection_stop( UINT mode );