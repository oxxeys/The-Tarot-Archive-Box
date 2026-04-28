const mongoose=require("mongoose")
const {Schema, model} = mongoose


const userSchema = new Schema({
    username: String,
    password: String,
    boxID: String,
})


const userData = model("users", userSchema)

//return bool if user is there
async function checkUser(username, password){
 

    let found = null
    found = await userData.findOne({username:username}).exec()

    if(found){
        return found.password==password // checks password - if it is same then it will return true
    } else{
        return false
    }
}

//add user to database
async function addNewUser(username, password, boxID){
 
    
    //check database to see if user already exists
    let findUser = await userData.findOne({username:username}).exec()

    if(findUser){
        return false
    }
    else{
        let newUserData = {
            username: username,
            password: password, 
            boxID: boxID
        }
        await userData.create(newUserData)
        return true
    }
}

module.exports={
    checkUser,
    addNewUser
}