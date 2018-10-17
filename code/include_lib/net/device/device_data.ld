
	 _ctp_mapping_tab_begin = .;
	PROVIDE(ctp_mapping_tab_begin = .);

	_ctp_mapping_tab_system_cmd_begin = .;
	PROVIDE(ctp_mapping_tab_system_cmd_begin = .);
		*(.ctp_system_cmd)
	_ctp_mapping_tab_system_cmd_end = .;
	PROVIDE(ctp_mapping_tab_system_cmd_end = .);

	_ctp_mapping_tab_video_cmd_begin = .;
	PROVIDE(ctp_mapping_tab_video_cmd_begin = .);
		*(.ctp_video_cmd)
	_ctp_mapping_tab_video_cmd_end = .;
	PROVIDE(ctp_mapping_tab_video_cmd_end = .);

	_ctp_mapping_tab_photo_cmd_begin = .;
	PROVIDE(ctp_mapping_tab_photo_cmd_begin = .);
		*(.ctp_photo_cmd)
	_ctp_mapping_tab_photo_cmd_end = .;
	PROVIDE(ctp_mapping_tab_photo_cmd_end = .);

	_ctp_mapping_tab_end = .;
	PROVIDE(ctp_mapping_tab_end = .);

	_sig_mapping_tab_cmd_begin = .;
	PROVIDE(sig_mapping_tab_cmd_begin = .);

