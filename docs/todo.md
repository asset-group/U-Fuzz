# 📋 Todo List 


<iframe 
:onload="finishLoading()" 
style="width: 100%; height: 75vh;" 
frameBorder="0" src="https://hackmd.io/@GvMKByasRLiK0vYm1YB2EA/HkA58Be0P" 
/>



<q-inner-loading :showing="innerLoadingVar">
<q-spinner size="100px" color="primary" />
</q-inner-loading>

<script>
export default {
	data() {
	    return {
	    	innerLoadingVar: true
	    }
	},
	methods:{
		finishLoading(){
			setTimeout(()=>{
				this.innerLoadingVar=false}
			, 1000);
		}
	}
}
</script>