const Logger = require('./logger')
const sendToLoggerQueue = require('./sendToLoggerQueue')
module.exports = async (event, context) => {
  const respEvent = { ...event }
  const currentStep = (context.functionName || '').replace(/^smart-dog-collar-app-test-(.*?)-/, '')
  // const skipStep = respEvent.stepHistory.includes(currentStep)
  console.log('CURRENT STEP: ', currentStep)
  // const logger = new Logger(respEvent, context)
  let logs = []

  const log = async (...logs) => {
    const { logStreamName } = event
    if(logStreamName){
      await sendToLoggerQueue(logStreamName, logs, currentStep)
    } else {
      console.log(logs)
    }
    return Promise.resolve()
  }
  const logNoLabel = async (...logs) => {
    const { logStreamName } = event
    if(logStreamName){
      await sendToLoggerQueue(logStreamName, logs)
    } else {
      console.log(logs)
    }
    return Promise.resolve()
  }
  
//   if(skipStep){
//     logs.push(`${currentStep} has already been executed`)
//   }else{
    // respEvent.stepHistory.push(currentStep)
    // logs.push('EVENT: ', respEvent)
    // logs.push('HISTORY: ', respEvent.stepHistory)

    await logNoLabel(`
----------------------------
- ${currentStep}
----------------------------
    `)
    
//   }
  await log(logs)
  return { respEvent, log }
  // return { respEvent, log, logger, skipStep }
}