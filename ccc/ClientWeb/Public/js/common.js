	function validate_form(form){
		var mail = $("#username").val();
		var pwd  = $("#password").val();
		var code  = $("#checkcode").val();
		if(!mail){
			$("#username").focus();
			$("#msg-username").show();
			return false;
		}
		if(!pwd){
			$("#password").focus();
			$("#msg-pwd").show();
			return false;
		}
		if(!code){
			$("#checkcode").focus();
			$("#msg-code").show();
			return false;
		}
		if(!isEmail(mail)){
			$("#username").focus();
			$("#msg-username").html("<span class='noempty'>*</span>输入正确的邮箱地址");
			$("#msg-username").show();
			return false;
		}
		return true;
	}
	function onBlue(){
		$("#msg-username").hide();
		$("#msg-code").hide();
		$("#msg-pwd").hide();
		$("#regist-error").hide();		
	}
	
	function isEmail(str){
		var myReg = /^[\w-]+(\.[\w-]+)*@[\w-]+(\.[\w-]+)+$/;
		if(myReg.test(str)) return true; 
		return false; 
	}
	
	
	
