{
  "targets": [
    {
      "target_name": "positivity",
      "sources": [ "positivity.cc"],
	  "include_dirs" : [
 	 			"<!(node -e \"require('nan')\")"
				]
    }
  ]
}
