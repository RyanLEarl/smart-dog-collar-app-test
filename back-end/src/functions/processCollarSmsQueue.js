const aws = require('aws-sdk')
const sqs = new aws.SQS({ region: 'us-west-2' })
const sns = new aws.SNS({apiVersion: '2010-03-31'})
const dig = require('./helpers/dig')

exports.handler = async (event) => {
  console.log('processCollarSmsQueue Invoked', JSON.stringify(event))
  const record = dig(event, 'Records', '0')
  if(!record){ return Promise.reject() }

  const { phone, message, timeout } = JSON.parse(record.body)
    console.log('Send SMS to:', phone)
    await sendSms(phone, message)
    return Promise.resolve()
}

const extendTimeout = async (id, timeout) => {
  console.log('Adding to message visibility:', Math.min(timeout, 43000))

  const params = {
    QueueUrl: process.env.woprSmsQueueUrl,
    ReceiptHandle: id,
    VisibilityTimeout: Math.min(timeout, 43000)
  };
  //console.log('params:', JSON.stringify(params))
  await sqs.changeMessageVisibility(params).promise()
    .then(data => console.log('visibility changed', data))
}

const sendSms = (PhoneNumber, Message) => 
  sns.publish({ 
    PhoneNumber, 
    Message,
    MessageAttributes: {
      'AWS.SNS.SMS.SMSType': {
        DataType: 'String',
        StringValue: 'Transactional'
      }
    }
  }).promise()
    .then(data => {
      console.log('text sent', data)
      return data
    })